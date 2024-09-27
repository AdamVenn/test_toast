#pragma once

namespace tst
{
using namespace juce;

class UnitTestApp;

// ============================================================================
class ConsoleLogger : public Logger
{
    void logMessage(const String& message) override
    {
        std::cout << message << std::endl;

#if JUCE_WINDOWS
        Logger::outputDebugString(message);
#endif
    }
};

// ============================================================================
class ConsoleUnitTestRunner : public UnitTestRunner
{
    void logMessage(const String& message) override
    {
        Logger::writeToLog(message);
    }
};

// ============================================================================
class BgTestRunner : public Thread
{
public:
    BgTestRunner();
    virtual void run();
    void setCategory(const String& categoryName);

private:
    friend class UnitTestApp;
    String m_categoryName{};
    bool readyToGo{ false };
    ConsoleUnitTestRunner runner;
    std::function<void()> finishedCallback;
};

// ============================================================================
/* Create a Unit Test Application with a GUI in order to run tests
on a background thread with a message thread running.
To use:
```
static MyTest tester{};
class MyUnitTest : public tst::UnitTestApp
{
public:
    MyUnitTest()
    {
        categoryName = "My unit test";
    }
};
START_JUCE_APPLICATION(MyUnitTest);
```
*/
class UnitTestApp : public JUCEApplicationBase
{
public:
    String applicationName{"Unit tests"};
    String applicationVersion{"1.0.0"};
    String categoryName{ "" };
    BgTestRunner runner{};
    ConsoleLogger logger;
    std::atomic<int> exitCode{ 0 };

    virtual const String getApplicationName();
    virtual const String getApplicationVersion();
    virtual bool moreThanOneInstanceAllowed();
    virtual void initialise(const String& commandLineParameters);
    virtual void shutdown();
    virtual void anotherInstanceStarted(const String& commandLine);
    virtual void systemRequestedQuit();
    virtual void suspended();
    virtual void resumed();
    virtual void unhandledException(const std::exception*, const String& sourceFilename, int lineNumber);

private:
    /* Called async when the tests have finished. Sets the exit code according to the result. */
    void testsFinished();
};

// ============================================================================
// Free functions

/* Usage: jassert(notOnMessageThread() */
bool notOnMessageThread();

/* Run a function on the message thread and wait for it to finish */
void synchronousAsync(auto funcToRun)
{
    jassert(notOnMessageThread());
    WaitableEvent event;
    auto decoratedFunc = [&]() {
        funcToRun();
        event.signal();
    };
    MessageManager::callAsync(decoratedFunc);
    event.wait();
}

void waitForMessageQueueToFinish();

/* Get a basic mouse event on a component. */
MouseEvent getMouseEvent(Component* eventComponent, uint8 numberOfClicks = 1);

} // tst
