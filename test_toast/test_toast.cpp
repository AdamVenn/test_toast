#include "test_toast.h"
namespace tst
{
// ============================================================================
// BgTestRunner
BgTestRunner::BgTestRunner() :
    Thread("Test Runner")
{
    runner.setAssertOnFailure(false);
}

void BgTestRunner::run()
{
    // Called start without setting category
    jassert(readyToGo);
    runner.runTestsInCategory(m_categoryName);
    MessageManager::callAsync(finishedCallback);
}

void BgTestRunner::setCategory(const String& categoryName)
{
    DBG("Background thread will run tests for category: " + categoryName);
    m_categoryName = categoryName;
    readyToGo = true;
}

// ============================================================================
// UnitTestApp
const String UnitTestApp::getApplicationName()
{
    return applicationName;
}

const String UnitTestApp::getApplicationVersion()
{
    return "1.0.0";
}

bool UnitTestApp::moreThanOneInstanceAllowed()
{
    return true;
}

void UnitTestApp::initialise(const String& /* commandLineParameters */)
{
    Logger::setCurrentLogger(&logger);
    logger.writeToLog(newLine + "Tests starting");
    runner.finishedCallback = [this]() {testsFinished(); };
    runner.setCategory(categoryName);
    runner.startThread();
}

void UnitTestApp::shutdown()
{
    runner.stopThread(1000);
    Logger::setCurrentLogger(nullptr);
}

void UnitTestApp::anotherInstanceStarted(const String& /* commandLine */)
{
}

void UnitTestApp::systemRequestedQuit()
{
}

void UnitTestApp::suspended()
{
}

void UnitTestApp::resumed()
{
}

void UnitTestApp::unhandledException(const std::exception*, const String& /* sourceFilename */, int /* lineNumber */)
{
}

void UnitTestApp::testsFinished()
{
    if (runner.runner.getNumResults() == 0)
    {
        logger.writeToLog("No tests were run.");
        exitCode = 1;
        shutdown();
        return;
    }

    std::vector<String> failures;

    for (int i = 0; i < runner.runner.getNumResults(); ++i)
    {
        auto* result = runner.runner.getResult(i);

        if (result->failures > 0) {
            failures.push_back(result->unitTestName + " / " + result->subcategoryName + ": " + String(result->failures) + " test failure" + (result->failures > 1 ? "s" : ""));
        }
    }

    if (!failures.empty())
    {
        logger.writeToLog(newLine + "Test failure summary:" + newLine);

        for (const auto& failure : failures)
            logger.writeToLog(failure);

        exitCode = 1;
        shutdown();
        return;
    }

    logger.writeToLog(newLine + "All tests completed successfully");
    shutdown();
}

// ============================================================================
// Free functions


bool notOnMessageThread()
{
    if (auto mm = MessageManager::getInstanceWithoutCreating())
    {
        return !mm->isThisTheMessageThread();
    }
    else
    {
        return false;
    }
}

void waitForMessageQueueToFinish()
{
    synchronousAsync([]() {
        ignoreUnused();
    });
}

MouseEvent getMouseEvent(Component* eventComponent, uint8 numberOfClicks)
{
    return MouseEvent{
        Desktop::getInstance().getMainMouseSource(), // MouseInputSource source,
        Point<float>{ 1.f, 1.f }, // Point<float> position,
        ModifierKeys{ 0 }, // ModifierKeys modifiers,
        1.f, // float pressure,
        0.f, 0.f, // float orientation, float rotation,
        0.f, 0.f, // float tiltX, float tiltY,
        eventComponent, // Component* eventComponent,
        nullptr, // Component* originator,
        Time{}, // Time eventTime,
        Point<float>{ 1.f, 1.f }, // Point<float> mouseDownPos,
        Time{}, // Time mouseDownTime,
        numberOfClicks, // int numberOfClicks (private member)
        false // bool mouseWasDragged
    };
}

} // namespace tst