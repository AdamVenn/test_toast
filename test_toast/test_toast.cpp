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
    runner.runTestsInCategory(m_categoryName);
    MessageManager::callAsync(finishedCallback);
}

void BgTestRunner::setCategory(const String &categoryName)
{
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
    quit();
}

void UnitTestApp::anotherInstanceStarted(const String &commandLine)
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

void UnitTestApp::unhandledException(const std::exception *, const String &sourceFilename, int lineNumber)
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

void tst::waitForMessageQueueToFinish()
{
    tst::synchronousAsync([]() {
        ignoreUnused();
        });
}

} // namespace tst