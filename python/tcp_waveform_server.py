"""
TCP 波形数据服务器
- 端口 502: 控制端口 (等待 QT 连接，可收发控制指令)
- 端口 503: 波形数据端口 (持续发送 1-16 通道波形数据)

数据包格式 (503 端口):
  每包包含 20 个时间点，每个时间点包含 16 个通道数据
  排列顺序: [CH1_pt0, CH2_pt0, ..., CH16_pt0, CH1_pt1, ..., CH16_pt19]
  每个数据点为小端序 4 字节浮点数 (float32)
  每包大小 = 20 * 16 * 4 = 1280 字节
"""

import asyncio
import json
import math
import os
import struct
from dataclasses import dataclass


# ============================================================
# 配置类
# ============================================================

@dataclass
class ServerConfig:
    """服务器配置"""

    host: str
    port_502_enabled: bool
    port_503_enabled: bool
    send_interval_ms: int
    samples_per_packet: int
    num_channels: int
    y_min: float
    y_max: float
    period: float
    amplitude: float
    phase_offset_per_channel: float
    noise_amplitude: float

    @classmethod
    def load_from_json(cls, path: str) -> "ServerConfig":
        """从 JSON 文件加载配置"""
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
        return cls(
            host=data["server"]["host"],
            port_502_enabled=data["server"]["port_502_enabled"],
            port_503_enabled=data["server"]["port_503_enabled"],
            send_interval_ms=data["waveform"]["send_interval_ms"],
            samples_per_packet=data["waveform"]["samples_per_packet"],
            num_channels=data["waveform"]["num_channels"],
            y_min=data["waveform"]["y_min"],
            y_max=data["waveform"]["y_max"],
            period=data["waveform"]["period"],
            amplitude=data["waveform"]["amplitude"],
            phase_offset_per_channel=data["waveform"]["phase_offset_per_channel"],
            noise_amplitude=data["waveform"]["noise_amplitude"],
        )

    @property
    def send_interval_sec(self) -> float:
        """发送间隔（秒）"""
        return self.send_interval_ms / 1000.0

    @property
    def total_points_per_packet(self) -> int:
        """每包数据点总数 = 时间点数 × 通道数"""
        return self.samples_per_packet * self.num_channels

    @property
    def packet_size_bytes(self) -> int:
        """每包字节大小 = 数据点总数 × 4"""
        return self.total_points_per_packet * 4

    def summary(self) -> str:
        """返回配置摘要字符串，用于启动时打印"""
        return (
            f"配置: 发送间隔={self.send_interval_ms}ms, "
            f"每包时间点数={self.samples_per_packet}, "
            f"通道数={self.num_channels}, "
            f"每包数据点总数={self.total_points_per_packet}, "
            f"每包大小={self.packet_size_bytes}字节, "
            f"Y范围=[{self.y_min}, {self.y_max}], "
            f"周期={self.period:.4f}, "
            f"振幅={self.amplitude}, "
            f"噪声={self.noise_amplitude}"
        )

    def port_status(self) -> str:
        """返回端口启用状态摘要"""
        return (
            f"端口 502 (控制): {'开启' if self.port_502_enabled else '关闭'}\n"
            f"端口 503 (波形): {'开启' if self.port_503_enabled else '关闭'}"
        )


# ============================================================
# 波形数据生成器
# ============================================================

class WaveformGenerator:
    """为 1-16 通道生成不同相位偏移的正弦波形数据"""

    def __init__(self, config: ServerConfig):
        self.cfg = config
        # 时间索引，用于推进波形相位
        self.time_index = 0.0
        # Y 轴中心值
        self.y_center = (self.cfg.y_max + self.cfg.y_min) / 2.0
        # 半振幅
        self.half_amp = self.cfg.amplitude / 2.0

    def _generate_point(self, sample_idx: int, ch: int) -> float:
        """
        生成单个数据点的值
        :param sample_idx: 当前数据包内的时间点索引 (0 ~ samples_per_packet-1)
        :param ch: 当前通道索引 (0 ~ num_channels-1)
        :return: 波形数值
        """
        # 每个通道有不同的相位偏移，使通道波形互相错开
        phase = ch * self.cfg.phase_offset_per_channel

        # 计算该数据点的"时间戳"：当前全局时间 + 包内偏移
        t = self.time_index + sample_idx * self.cfg.send_interval_sec

        # 基础正弦波
        sine = math.sin(2.0 * math.pi * t / self.cfg.period + phase)

        # 噪声：基于时间和通道的伪随机扰动（使用正弦叠加，避免使用 random 破坏确定性）
        noise = self.cfg.noise_amplitude * math.sin(
            self.time_index * 3.7 + ch * 1.3 + sample_idx * 0.1
        )

        # 合成并钳位到 Y 轴范围
        value = self.y_center + self.half_amp * sine + noise
        return max(self.cfg.y_min, min(self.cfg.y_max, value))

    def generate_packet(self) -> bytes:
        """
        生成一个完整数据包
        排列: [CH1_pt0, CH2_pt0, ..., CH16_pt0, CH1_pt1, ..., CH16_pt19]
        """
        values = []
        for sample_idx in range(self.cfg.samples_per_packet):
            for ch in range(self.cfg.num_channels):
                values.append(self._generate_point(sample_idx, ch))

        # 推进全局时间索引，供下一包使用
        self.time_index += self.cfg.samples_per_packet * self.cfg.send_interval_sec

        # 打包为小端序 float32 字节流
        return struct.pack(f"<{len(values)}f", *values)


# ============================================================
# TCP 波形服务器主类
# ============================================================

class TcpWaveformServer:
    """管理两个 TCP 端口（502 控制、503 波形）的主服务"""

    PORT_CONTROL = 502
    PORT_WAVEFORM = 503

    def __init__(self, config: ServerConfig):
        self.cfg = config
        self.generator = WaveformGenerator(config)

        # 两个端口的已连接客户端集合
        self.control_clients: set[asyncio.StreamWriter] = set()
        self.waveform_clients: set[asyncio.StreamWriter] = set()

        # asyncio Server 对象
        self._control_server: asyncio.AbstractServer | None = None
        self._waveform_server: asyncio.AbstractServer | None = None

        # 波形发送协程任务
        self._sender_task: asyncio.Task | None = None

    # --------------------------------------------------------
    # 启动
    # --------------------------------------------------------

    async def start(self) -> bool:
        """启动所有启用的端口。返回是否至少有一个端口成功启动"""
        if self.cfg.port_502_enabled:
            self._control_server = await self._start_port(
                self.PORT_CONTROL, self._handle_control_client
            )

        if self.cfg.port_503_enabled:
            self._waveform_server = await self._start_port(
                self.PORT_WAVEFORM, self._handle_waveform_client
            )

        # 如果有波形端口启动，启动数据发送循环
        if self._waveform_server is not None:
            self._sender_task = asyncio.create_task(self._waveform_sender_loop())

        any_started = (
            self._control_server is not None    or 
            self._waveform_server is not None
        )
        if not any_started:
            print("没有可用的服务器启动，请检查 config.json 中的端口启用设置")

        return any_started

    async def _start_port(self, port: int, handler) -> asyncio.AbstractServer | None:
        """启动单个端口的监听"""
        try:
            server = await asyncio.start_server(handler, self.cfg.host, port)
            print(f"[{port}] 服务器已启动 {self.cfg.host}:{port}")
            return server
        except OSError as e:
            print(f"[{port}] 端口启动失败: {e}")
            return None

    # --------------------------------------------------------
    # 客户端处理
    # --------------------------------------------------------

    async def _handle_control_client(
        self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
    ):
        """502 端口 - 控制端口：接收客户端连接并回显收到的数据"""
        addr = writer.get_extra_info("peername")
        self.control_clients.add(writer)
        print(f"[502] 客户端已连接: {addr}")

        try:
            while True:
                data = await reader.read(4096)
                if not data:
                    break
                print(f"[502] 收到数据 {len(data)} 字节: {data[:64]}")
        except (ConnectionResetError, asyncio.CancelledError):
            pass
        finally:
            self.control_clients.discard(writer)
            writer.close()
            await writer.wait_closed()
            print(f"[502] 客户端已断开: {addr}")

    async def _handle_waveform_client(
        self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
    ):
        """503 端口 - 波形数据端口：接收客户端连接并由发送循环向其推送数据"""
        addr = writer.get_extra_info("peername")
        self.waveform_clients.add(writer)
        print(f"[503] 客户端已连接: {addr}")

        try:
            # 此循环仅用于检测客户端断开（客户端主动关闭时 read 返回空）
            while True:
                data = await reader.read(4096)
                if not data:
                    break
        except (ConnectionResetError, asyncio.CancelledError):
            pass
        finally:
            self.waveform_clients.discard(writer)
            writer.close()
            await writer.wait_closed()
            print(f"[503] 客户端已断开: {addr}")

    async def _waveform_sender_loop(self):
        """持续向所有 503 端口客户端发送波形数据包"""
        while True:
            if self.waveform_clients:
                packet = self.generator.generate_packet()
                dead_clients: list[asyncio.StreamWriter] = []

                for writer in self.waveform_clients:
                    try:
                        writer.write(packet)
                        await writer.drain()
                    except (ConnectionResetError, BrokenPipeError, OSError):
                        dead_clients.append(writer)

                # 清理已断开的客户端
                for w in dead_clients:
                    self.waveform_clients.discard(w)
                    try:
                        w.close()
                        await w.wait_closed()
                    except Exception:
                        pass
            else:
                # 无客户端时仍推进时间索引，保持波形时间连续
                self.generator.time_index += (
                    self.cfg.samples_per_packet * self.cfg.send_interval_sec
                )

            await asyncio.sleep(self.cfg.send_interval_sec)

    # --------------------------------------------------------
    # 关闭
    # --------------------------------------------------------

    async def stop(self):
        """停止所有端口和协程"""
        if self._sender_task and not self._sender_task.done():
            self._sender_task.cancel()

        for srv in (self._control_server, self._waveform_server):
            if srv is not None:
                srv.close()
                await srv.wait_closed()

        for w in self.control_clients | self.waveform_clients:
            try:
                w.close()
                await w.wait_closed()
            except Exception:
                pass

        self._control_server = None
        self._waveform_server = None
        print("服务器已停止")

    # --------------------------------------------------------
    # 运行直到被中断
    # --------------------------------------------------------

    async def run_until_stopped(self):
        """运行服务直到收到 CancelledError（通常来自 Ctrl+C）"""
        try:
            # 通过 gather 等待所有 server 关闭（正常情况下永远运行）
            tasks = []
            if self._control_server is not None:
                tasks.append(self._control_server.serve_forever())
            if self._waveform_server is not None:
                tasks.append(self._waveform_server.serve_forever())
            if tasks:
                await asyncio.gather(*tasks)
        except asyncio.CancelledError:
            pass


# ============================================================
# 主入口
# ============================================================

def _get_config_path() -> str:
    """获取与本脚本同目录下的 config.json 路径"""
    return os.path.join(os.path.dirname(os.path.abspath(__file__)), "config.json")


async def main():
    config = ServerConfig.load_from_json(_get_config_path())
    server = TcpWaveformServer(config)

    if not await server.start():
        return

    print("=" * 50)
    print(config.port_status())
    print(config.summary())
    print("=" * 50)
    print("服务器运行中，按 Ctrl+C 停止...")

    try:
        await server.run_until_stopped()
    finally:
        await server.stop()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n服务器已关闭")
