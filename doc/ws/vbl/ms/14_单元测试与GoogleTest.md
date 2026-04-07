# 14 · 单元测试与 GoogleTest

> **面试价值**：⭐⭐⭐  **优先级**：P2
> **相关文件**：
> - tests/ — 测试文件目录
> - modules/*/Test/ — 各模块测试文件
> - CMakeLists.txt — AppendModulesBasic 中的测试 target 自动生成

---

## 1. GoogleTest 基本用法

VBL 使用 GoogleTest（GTest）作为单元测试框架：

    // 普通测试：TEST(TestSuite, TestName)
    TEST(DmTimelineTest, AddClipShouldIncreaseClipCount) {
        // Arrange：准备测试数据
        auto timeline = CreateTestTimeline();
        auto clip = CreateTestVideoClip();
        
        // Act：执行被测操作
        Result r = timeline->AddClip(clip, 0, 0);
        
        // Assert：验证结果
        EXPECT_EQ(r, Result::rOk);
        EXPECT_EQ(timeline->GetClipCount(0), 1);
    }
    
    // 夹具测试：TEST_F(FixtureClass, TestName)
    class BsUndoManagerTest : public ::testing::Test {
    protected:
        void SetUp() override {
            m_undoManager = std::make_unique<BsUndoManager>();
            m_timeline = CreateTestTimeline();
        }
        void TearDown() override {
            m_undoManager.reset();
        }
        std::unique_ptr<BsUndoManager> m_undoManager;
        SafePtr<IDmTimeline> m_timeline;
    };
    
    TEST_F(BsUndoManagerTest, UndoShouldRevertLastOperation) {
        // 执行操作并注册 Undo
        auto clip = CreateTestVideoClip();
        m_timeline->AddClip(clip, 0, 0);
        m_undoManager->Push(new BsUndoAutoTemplateCommand("Add",
            [&]() { m_timeline->AddClip(clip, 0, 0); },
            [&]() { m_timeline->RemoveClip(clip->GetId()); }
        ));
        
        EXPECT_EQ(m_timeline->GetClipCount(0), 1);
        
        // 撤销
        m_undoManager->Undo();
        EXPECT_EQ(m_timeline->GetClipCount(0), 0);
    }

---

## 2. 参数化测试（TEST_P）

文件：modules/Adapter/wes/Test/TrackAdapter_test.cpp（参数化测试示意）

参数化测试用于同一测试逻辑覆盖多组输入：

    // 参数类型定义
    struct TrackTestParam {
        int trackType;
        int expectedMaxClips;
        std::string description;
    };
    
    // 夹具继承 TestWithParam
    class TrackAdapterTest
        : public ::testing::TestWithParam<TrackTestParam> {
    protected:
        void SetUp() override {
            m_adapter = CreateTrackAdapter();
        }
        std::shared_ptr<ITrackAdapter> m_adapter;
    };
    
    // 参数化测试体（GetParam() 获取当前参数）
    TEST_P(TrackAdapterTest, MaxClipsMatchesTrackType) {
        const auto& param = GetParam();
        
        m_adapter->SetTrackType(param.trackType);
        int actual = m_adapter->GetMaxClipCount();
        
        EXPECT_EQ(actual, param.expectedMaxClips)
            << "For track type: " << param.description;
    }
    
    // 实例化：提供参数集合
    INSTANTIATE_TEST_SUITE_P(
        AllTrackTypes,
        TrackAdapterTest,
        ::testing::Values(
            TrackTestParam{0, 100, "VideoTrack"},
            TrackTestParam{1, 100, "AudioTrack"},
            TrackTestParam{2,  10, "EffectTrack"},
            TrackTestParam{3,   1, "TextTrack"}
        )
    );
    
    // 自动生成4个测试用例：
    // AllTrackTypes/TrackAdapterTest.MaxClipsMatchesTrackType/0
    // AllTrackTypes/TrackAdapterTest.MaxClipsMatchesTrackType/1
    // AllTrackTypes/TrackAdapterTest.MaxClipsMatchesTrackType/2
    // AllTrackTypes/TrackAdapterTest.MaxClipsMatchesTrackType/3

---

## 3. CMake 自动生成测试 target

AppendModulesBasic 函数中集成了测试 target 的自动创建：

    # CMakeLists.txt（AppendModulesBasic 中的测试部分）
    if(VBL_BUILD_TESTS AND EXISTS "${MODULE_NAME}/Test")
        file(GLOB_RECURSE TEST_FILES "${MODULE_NAME}/Test/*.cpp")
        
        # 创建测试可执行文件
        add_executable(${MODULE_NAME}_test ${TEST_FILES})
        
        # 链接被测模块和 GTest
        target_link_libraries(${MODULE_NAME}_test
            PRIVATE
                ${MODULE_NAME}      # 被测模块
                GTest::gtest_main   # GTest + main()
        )
        
        # 注册测试（CTest 集成）
        gtest_discover_tests(${MODULE_NAME}_test
            OUTPUT_DIR "${CMAKE_BINARY_DIR}/test-results"
            EXTRA_ARGS "--gtest_output=json:${MODULE_NAME}_results.json"
        )
    endif()
    
    # 使用：只要模块有 Test/ 目录，测试 target 自动创建
    AppendModulesBasic(BsProxy)    # 自动创建 BsProxy_test
    AppendModulesBasic(BsUndoManager)  # 自动创建 BsUndoManager_test

---

## 4. 测试输出 JSON 格式（CI 集成）

    # 运行测试并输出 JSON（CI/CD 流水线中使用）
    ./BsProxy_test --gtest_output=json:BsProxy_results.json
    
    # 输出格式示例
    {
      "tests": 12,
      "failures": 0,
      "disabled": 0,
      "errors": 0,
      "time": "0.045s",
      "testsuites": [
        {
          "name": "BsProxyTest",
          "tests": 4,
          "testsuite": [
            { "name": "ProxyPathExists",    "status": "RUN", "result": "COMPLETED", "time": "0.003s" },
            { "name": "ProxyPathNotExists", "status": "RUN", "result": "COMPLETED", "time": "0.001s" }
          ]
        }
      ]
    }

---

## 5. 常用 GTest 断言

    EXPECT_EQ(a, b)     // 期望 a == b，失败时继续执行
    EXPECT_NE(a, b)     // 期望 a != b
    EXPECT_TRUE(cond)   // 期望条件为 true
    EXPECT_FALSE(cond)  // 期望条件为 false
    EXPECT_STREQ(s1, s2) // 字符串相等
    EXPECT_NEAR(a, b, delta)  // 浮点数近似相等
    
    ASSERT_EQ(a, b)     // 断言 a == b，失败时立即终止当前测试
    ASSERT_NE(a, b)
    ASSERT_TRUE(cond)
    
    // 期望死亡（测试异常/断言场景）
    EXPECT_DEATH(code_that_crashes(), "error message regex");

---

## 6. 面试要点

1. **TEST vs TEST_F vs TEST_P 的选择**：
   TEST：简单的独立测试，不需要共享状态；
   TEST_F：需要 SetUp/TearDown 共享测试夹具（如初始化一个复杂对象）；
   TEST_P：同一测试逻辑需要多组参数（如测试不同轨道类型、不同媒体格式）。

2. **参数化测试消除测试代码重复**：TrackAdapter_test.cpp 用 TEST_P 一次定义测试，
   INSTANTIATE_TEST_SUITE_P 提供4种参数，自动生成4个独立测试用例，
   比写4个几乎相同的 TEST 函数更简洁，也更容易添加新的参数组合。

3. **AppendModulesBasic 中集成测试 target 实现了"测试与模块同步"**：
   每个模块都有对应的 _test target，开发者在模块目录下的 Test/ 子目录写测试，
   无需额外的 CMake 配置，降低了写测试的门槛。

4. **JSON 输出为 CI 集成提供了机器可读结果**：Jenkins/GitLab CI 可以解析
   gtest_output=json 格式的测试报告，展示测试通过率、历史趋势，失败时精确定位。

5. **EXPECT_ vs ASSERT_ 的使用策略**：用 EXPECT_ 允许测试继续收集更多失败信息；
   用 ASSERT_ 当后续步骤依赖当前检查结果时（如 ASSERT_NE(ptr, nullptr) 后才能
   安全解引用）。一般情况下优先用 EXPECT_，避免 ASSERT_ 中断导致漏报其他失败。

---

## 7. 可能被追问的问题

**Q1：如何对单例模式的代码进行单元测试？**
问题：单例全局状态在测试间共享，前一个测试的状态会影响后一个。
方案1：在单例上暴露 Reset() 方法（只在 VBL_TESTING 宏下编译），TearDown 时重置。
方案2：依赖注入——业务代码通过接口接收依赖，测试时注入 mock 对象而非单例。
VBL 中 BsProxyFileName::Instance() 这类工具类单例一般不需要 mock，
只有有业务逻辑的单例（如 VbPreferenceManager）才需要注意测试隔离。

**Q2：如何测试包含多线程的代码？**
策略1：注入执行器（Executor）——把线程逻辑抽象为接口，测试时注入同步执行器。
策略2：等待完成——提交任务后调用 WaitAll()，然后验证结果（BsBackgroundTaskManager 有此接口）。
策略3：sleep + retry（不推荐，flaky test）。
VBL 的测试倾向于策略2，提交后台任务后 WaitAll()，再用 EXPECT 验证结果。

**Q3：GTest 的 EXPECT 和 ASSERT 失败信息如何自定义？**
用 << 操作符追加失败信息：
EXPECT_EQ(actual, expected) << "Context: " << contextInfo;
失败时会打印额外的 Context 信息，帮助诊断。

**Q4：如何测试私有方法？**
方案1：通过公有方法间接测试（最推荐，测试行为而非实现）。
方案2：将私有方法提取到独立类（更好的设计，可单独测试）。
方案3：friend 测试类（在头文件中声明 FRIEND_TEST(TestSuite, TestName)，不推荐，污染接口）。
VBL 的测试原则：通过公共接口测试，私有实现的正确性由公共接口的测试间接验证。
