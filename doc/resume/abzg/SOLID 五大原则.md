# 一、SOLID 五大原则（核心）

## 1. 单一职责原则（SRP, Single Responsibility Principle）

> 一个类只做一件事，并且只有一个引起它变化的原因。

**理解重点：**

- 不要“万能类”
- 一个类职责越多，修改风险越大

**例子：**
 ❌ 一个类既负责“用户逻辑”，又负责“数据库操作”，还负责“日志”
 ✅ 拆分：

- UserService（业务）
- UserRepository（数据）
- Logger（日志）

------

## 2. 开闭原则（OCP, Open/Closed Principle）

> 对扩展开放，对修改关闭

**理解重点：**

- 允许新增功能
- 尽量不要改旧代码

**常见实现方式：**

- 接口 + 多态
- 策略模式

**例子：**
 新增支付方式：

- 不改原 Payment 逻辑
- 只新增 Alipay / WechatPay 类

------

## 3. 里氏替换原则（LSP, Liskov Substitution Principle）

> 子类必须能够替换父类并正确工作

**理解重点：**

- 不能破坏父类语义
- 子类不能“缩小能力”或“改变行为逻辑”

**反例：**
 如果 Bird 有 fly()
 但 Penguin 不能飞 → 继承 Bird 就不合理

👉 解决：

- 拆接口：IFlyable

------

## 4. 接口隔离原则（ISP, Interface Segregation Principle）

> 不要强迫类实现它不需要的接口

**理解重点：**

- 接口要“小而专”
- 比“胖接口”更灵活

**例子：**
 ❌ 一个 IWorker 接口：

- work()
- eat()
- sleep()

👉 机器人不需要 eat/sleep

✅ 拆分：

- IWorkable
- IFeedable
- ISleepable

------

## 5. 依赖倒置原则（DIP, Dependency Inversion Principle）

> 高层模块不依赖低层模块，二者都依赖抽象

**理解重点：**

- 依赖接口，而不是具体实现
- 面向抽象编程

**例子：**
 ❌
 OrderService 直接依赖 MySQLDatabase

✅
 OrderService → IDatabase
 MySQL / MongoDB 只是实现

------

# 二、常见补充设计原则

## 1. DRY（Don’t Repeat Yourself）

- 不要重复代码
- 逻辑统一封装

------

## 2. KISS（Keep It Simple, Stupid）

- 设计越简单越好
- 不要过度设计

------

## 3. YAGNI（You Aren’t Gonna Need It）

- 不要写“未来可能用到”的功能

------

## 4. LoD（迪米特法则 / 最少知识原则）

> 一个对象只和“直接朋友”通信

**核心：**

- 不要链式调用太深
- 降低耦合

❌ a.getB().getC().doSomething()

------

# 三、一句话总结（面试高频）

> 面向对象设计的核心目标是：
>  **高内聚、低耦合、可扩展、可复用、易维护**

------

# 四、面试加分理解（很重要）

如果你想说得更“高级”，可以补一句：

> SOLID 本质是在解决：

- 变化的隔离（OCP / DIP）
- 复杂度控制（SRP / ISP）
- 行为正确性（LSP）