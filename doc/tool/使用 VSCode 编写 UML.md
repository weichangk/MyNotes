

### WIN 环境准备

1. 安装java（PlantUML 运行依赖）

   - https://www.oracle.com/java/technologies/downloads/
   - 系统环境变量新增
      - JAVA_HOME为jdk安装的目录
      - CLASSPATH为`.;%JAVA_HOME%\lib;%JAVA_HOME%\lib\tools.jar`

   - 系统环境path变量编辑新增
     - `%JAVA_HOME%\bin;%JAVA_HOME%\jre\bin`

2. 安装 GraphViz（用于渲染图形）

   - https://plantuml.com/zh/starting

### MAC 环境准备

1. 安装 Java

   PlantUML 需要 Java 运行环境支持，建议使用 JDK 8 或更高版本。

   - 使用 Homebrew 安装：

     ```
     brew install openjdk
     ```

   - 设置 JAVA_HOME

     ```
     echo 'export PATH="/opt/homebrew/opt/openjdk/bin:$PATH"' >> ~/.zprofile
     source ~/.zprofile
     ```

   - 确认安装成功

     ```
     java -version
     ```

2. 安装 Graphviz

   PlantUML 中的类图、时序图等依赖 Graphviz 渲染。

   ```
   brew install graphviz
   # 确认安装成功：
   dot -V
   ```

   

### 安装 VSCode 插件

搜索 `PlantUML`，安装由 **jebbs** 提供的插件



### 创建 UML 文件并预览

1. 新建 `.puml` 文件

   ```
   @startuml
   class Person {
     - name: String
     - age: int
     + getInfo(): String
   }
   
   class Student {
     - school: String
     + study(): void
   }
   
   Person <|-- Student
   @enduml
   ```

2. 预览 UML 图

   命令面板：输入 `PlantUML: Preview Current Diagram` 并回车

3. 导出图片（PNG、SVG、PDF）

   命令面板：输入`PlantUML: Export Current Diagram` 在弹出的格式选择中选择 `png/svg/pdf` 即可导出

### 可选增强体验

1. 使用 Markdown 集成 Mermaid（轻量方案）

   新建 `.md` 文件写入以下代码也可以画 UML 类图：

   ```
   ```mermaid
   classDiagram
     class Person {
       -name: String
       -age: int
       +getInfo(): String
     }
   
     class Student {
       -school: String
       +study(): void
     }
   
     Person <|-- Student
   ```

   安装插件 `Markdown Preview Enhanced` 或 `Mermaid Markdown Syntax Highlighting`

   使用 VSCode 的 Markdown 预览功能（`Cmd+Shift+V`）即可查看图形

2. 推荐插件组合

   | 插件名称                  | 作用                       |
   | ------------------------- | -------------------------- |
   | PlantUML (jebbs)          | 编辑、预览、导出 UML 图    |
   | Markdown Preview Enhanced | 支持 Mermaid 图预览        |
   | Draw.io Integration       | 拖拽方式绘图（图形化操作） |
   | Java Extension Pack       | Java 支持，间接增强兼容    |