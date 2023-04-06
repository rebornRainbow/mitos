### 这是json的配置文件及其参数解释
{
  "version": "0.2.0",
  "configurations": [
      {
          "name": "debug xv6",//名字
          "preLaunchTask": "make-gdb",//debug 之前执行的任务对应task.json
          "type": "cppdbg",//类型
          "request": "launch",//不同的类型 launch和attach两种看文档
          "program": "${workspaceFolder}/kernel/kernel",//需要debug的程序
          "args": [],//传递给程序的参数
          "stopAtEntry": true,//在进入点暂停
          "cwd": "${workspaceFolder}",//当前的工作目录
          "miDebuggerServerAddress": "localhost:26000",//服务器
          "miDebuggerPath": "/usr/bin/gdb-multiarch",//gdb的路径
          "environment": [],
          "externalConsole": false,//出现一个控制台
          "MIMode": "gdb",//那种gdb方式可以是lldb
          "setupCommands": [
              {
                  "description": "pretty printing",
                  "text": "-enable-pretty-printing",
                  "ignoreFailures": true
              }
          ],
          "logging": {
              // "engineLogging": true,
              // "programOutput": true,
          }
      }
  ]
}

### 这是task.json文件及其解释
{
  "tasks": [
    {
      "label": "make-gdb",
      "command": "make",
      "args": [
        "qemu-gdb"
      ],
      "isBackground": true
    },
  ],
  "version": "2.0.0"
}