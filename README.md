# MotolControlQml

基于 Qt 6 Quick (QML) 的 TCP 连接服务器数据可视化应用程序。通过 TCP 协议连接远程服务器，接收数据并以图形化方式实时展示。

## 功能特性

- TCP 客户端连接服务器并接收数据
- 数据可视化显示
- IP 设置与连接状态管理
- 自定义开关组件（带动画效果）
- 基于 Qt Design Studio 的 UI 设计

## 技术栈

- **Qt 6.8** (Core, Gui, Widgets, Qml, Quick, QuickTimeline, ShaderTools)
- **CMake 3.21.1+**
- **Qt Design Studio 4.8**
- **QML / Qt Quick**

## 项目结构

```
MotolControlQml/
├── App/                          # 应用程序入口
│   ├── main.cpp                  # 主函数，QML 引擎加载
│   ├── autogen/
│   │   └── environment.h         # Qt Design Studio 自动生成的环境配置
│   └── CMakeLists.txt
├── MotolControlQml/              # QML 模块（常量、事件模拟）
│   ├── Constants.qml             # 全局常量（窗口尺寸、颜色、字体）
│   ├── EventListModel.qml        # 事件列表模型
│   ├── EventListSimulator.qml    # 事件模拟器
│   ├── qmldir                    # QML 模块定义
│   ├── designer/
│   │   └── plugin.metainfo       # Qt Design Studio 插件元信息
│   └── CMakeLists.txt
├── MotolControlQmlContent/       # 主要 UI 内容
│   ├── App.qml                   # 应用主窗口
│   ├── Screen01.ui.qml           # 主屏幕（左右布局）
│   ├── LeftPanel.ui.qml          # 左侧面板（IP设置、连接状态）
│   ├── SwitchMy.ui.qml           # 自定义开关组件
│   ├── fonts/                    # 字体资源
│   ├── images/                   # 图片资源
│   └── CMakeLists.txt
├── Dependencies/                 # Qt Design Studio 组件依赖
│   └── Components/               # Studio 组件库
│       └── imports/              # 各类 QML 组件导入
├── cmake/
│   └── insight.cmake             # Qt Insight 配置
├── CMakeLists.txt                # 根 CMake 配置
├── qds.cmake                     # Qt Design Studio 子目录配置
├── MotolControlQml.qmlproject    # Qt Design Studio 项目文件
├── MotolControlQml.qrc           # Qt 资源文件
└── qtquickcontrols2.conf         # Qt Quick Controls 配置
```

## 构建与运行

### 环境要求

- Qt 6.8 或更高版本
- CMake 3.21.1 或更高版本
- C++17 兼容编译器（MSVC / GCC / Clang）


### 使用 Qt Creator

直接用 Qt Creator 打开根目录的 `CMakeLists.txt` 即可。

### 使用 Qt Design Studio

打开 `MotolControlQml.qmlproject` 文件进行 UI 编辑。