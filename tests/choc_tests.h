#ifndef CHOC_TESTS_HEADER_INCLUDED
#define CHOC_TESTS_HEADER_INCLUDED
#include "../FileDump.hpp"
#include "../FileWatch.hpp"
#include "../StringUtils.hpp"
#include "../TextTable.hpp"
#include "../UTF8.hpp"
#include "../glob.hpp"

#include "choc_UnitTest.h"
#include <format>
#include <future>

#include "choc_MessageLoop.hpp"
/**
    To keep things simple for users, I've just shoved all the tests for everything into this
    one dependency-free header, and provided one function call (`choc::test::runAllTests) that
    tests everything.

    The idea is that you can then simply include this header and call runAllTests() somewhere in
    your own test build, to make sure that everything is working as expected in your project.

    At some point the library will probably grow to a size where this needs to be refactored into
    smaller modules and done in a more sophisticated way, but we're not there yet!
*/
namespace choc_unit_tests
{

/// Just create a TestProgress and pass it to this function to run all the
/// tests. The TestProgress object contains a callback that will be used
/// to log its progress.
bool runAllTests(choc::test::TestProgress &);
/// Performs the setup function, then waits for it to call the exit function provided,
/// then calls handleResult

/// Performs the setup function, then waits for it to call the exit function provided,
/// then calls handleResult
static void runTestOnMessageThread(std::function<void(const std::function<void()> &)> setup,
                                   std::function<void()>                              handleResult = {})
{
	std::atomic_bool      finished{false};
	std::function<void()> setFinished = [&] { finished = true; };

	choc::messageloop::postMessage([&] {
		setup(setFinished);
	});

	while (!finished)
		std::this_thread::yield();

	if (handleResult)
	{
		finished = false;

		choc::messageloop::postMessage([&] {
			handleResult();
			finished = true;
		});

		while (!finished)
			std::this_thread::yield();
	}
}

//==============================================================================

inline void testStringUtilities(choc::test::TestProgress &progress)
{
	CHOC_CATEGORY(Strings);


	{
		CHOC_TEST(Trimming)

		CHOC_EXPECT_EQ("test", choc::text::trim("test"));
		CHOC_EXPECT_EQ("test", choc::text::trim(" test"));
		CHOC_EXPECT_EQ("test", choc::text::trim("  test"));
		CHOC_EXPECT_EQ("test", choc::text::trim("test  "));
		CHOC_EXPECT_EQ("test", choc::text::trim("test "));
		CHOC_EXPECT_EQ("test", choc::text::trim("  test  "));
		CHOC_EXPECT_EQ("", choc::text::trim("  "));
		CHOC_EXPECT_EQ("", choc::text::trim(" "));
		CHOC_EXPECT_EQ("", choc::text::trim(""));

		CHOC_EXPECT_EQ("test", choc::text::trim(std::string_view("test")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string_view(" test")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string_view("  test")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string_view("test  ")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string_view("test ")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string_view("  test  ")));
		CHOC_EXPECT_EQ("", choc::text::trim(std::string_view("  ")));
		CHOC_EXPECT_EQ("", choc::text::trim(std::string_view(" ")));
		CHOC_EXPECT_EQ("", choc::text::trim(std::string_view("")));

		CHOC_EXPECT_EQ("test", choc::text::trim(std::string("test")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string(" test")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string("  test")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string("test  ")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string("test ")));
		CHOC_EXPECT_EQ("test", choc::text::trim(std::string("  test  ")));
		CHOC_EXPECT_EQ("", choc::text::trim(std::string("  ")));
		CHOC_EXPECT_EQ("", choc::text::trim(std::string(" ")));
		CHOC_EXPECT_EQ("", choc::text::trim(std::string("")));

		CHOC_EXPECT_EQ("test", choc::text::trimStart("test"));
		CHOC_EXPECT_EQ("test", choc::text::trimStart(" test"));
		CHOC_EXPECT_EQ("test", choc::text::trimStart("  test"));
		CHOC_EXPECT_EQ("test  ", choc::text::trimStart("test  "));
		CHOC_EXPECT_EQ("test ", choc::text::trimStart("test "));
		CHOC_EXPECT_EQ("test  ", choc::text::trimStart("  test  "));
		CHOC_EXPECT_EQ("", choc::text::trimStart("  "));
		CHOC_EXPECT_EQ("", choc::text::trimStart(" "));
		CHOC_EXPECT_EQ("", choc::text::trimStart(""));

		CHOC_EXPECT_EQ("test", choc::text::trimStart(std::string_view("test")));
		CHOC_EXPECT_EQ("test", choc::text::trimStart(std::string_view(" test")));
		CHOC_EXPECT_EQ("test", choc::text::trimStart(std::string_view("  test")));
		CHOC_EXPECT_EQ("test  ", choc::text::trimStart(std::string_view("test  ")));
		CHOC_EXPECT_EQ("test ", choc::text::trimStart(std::string_view("test ")));
		CHOC_EXPECT_EQ("test  ", choc::text::trimStart(std::string_view("  test  ")));
		CHOC_EXPECT_EQ("", choc::text::trimStart(std::string_view("  ")));
		CHOC_EXPECT_EQ("", choc::text::trimStart(std::string_view(" ")));
		CHOC_EXPECT_EQ("", choc::text::trimStart(std::string_view("")));

		CHOC_EXPECT_EQ("test", choc::text::trimStart(std::string("test")));
		CHOC_EXPECT_EQ("test", choc::text::trimStart(std::string(" test")));
		CHOC_EXPECT_EQ("test", choc::text::trimStart(std::string("  test")));
		CHOC_EXPECT_EQ("test  ", choc::text::trimStart(std::string("test  ")));
		CHOC_EXPECT_EQ("test ", choc::text::trimStart(std::string("test ")));
		CHOC_EXPECT_EQ("test  ", choc::text::trimStart(std::string("  test  ")));
		CHOC_EXPECT_EQ("", choc::text::trimStart(std::string("  ")));
		CHOC_EXPECT_EQ("", choc::text::trimStart(std::string(" ")));
		CHOC_EXPECT_EQ("", choc::text::trimStart(std::string("")));

		CHOC_EXPECT_EQ("test", choc::text::trimEnd("test"));
		CHOC_EXPECT_EQ(" test", choc::text::trimEnd(" test"));
		CHOC_EXPECT_EQ("  test", choc::text::trimEnd("  test"));
		CHOC_EXPECT_EQ("test", choc::text::trimEnd("test  "));
		CHOC_EXPECT_EQ("test", choc::text::trimEnd("test "));
		CHOC_EXPECT_EQ("  test", choc::text::trimEnd("  test  "));
		CHOC_EXPECT_EQ("", choc::text::trimEnd("  "));
		CHOC_EXPECT_EQ("", choc::text::trimEnd(" "));
		CHOC_EXPECT_EQ("", choc::text::trimEnd(""));

		CHOC_EXPECT_EQ("test", choc::text::trimEnd(std::string_view("test")));
		CHOC_EXPECT_EQ(" test", choc::text::trimEnd(std::string_view(" test")));
		CHOC_EXPECT_EQ("  test", choc::text::trimEnd(std::string_view("  test")));
		CHOC_EXPECT_EQ("test", choc::text::trimEnd(std::string_view("test  ")));
		CHOC_EXPECT_EQ("test", choc::text::trimEnd(std::string_view("test ")));
		CHOC_EXPECT_EQ("  test", choc::text::trimEnd(std::string_view("  test  ")));
		CHOC_EXPECT_EQ("", choc::text::trimEnd(std::string_view("  ")));
		CHOC_EXPECT_EQ("", choc::text::trimEnd(std::string_view(" ")));
		CHOC_EXPECT_EQ("", choc::text::trimEnd(std::string_view("")));

		CHOC_EXPECT_EQ("test", choc::text::trimEnd(std::string("test")));
		CHOC_EXPECT_EQ(" test", choc::text::trimEnd(std::string(" test")));
		CHOC_EXPECT_EQ("  test", choc::text::trimEnd(std::string("  test")));
		CHOC_EXPECT_EQ("test", choc::text::trimEnd(std::string("test  ")));
		CHOC_EXPECT_EQ("test", choc::text::trimEnd(std::string("test ")));
		CHOC_EXPECT_EQ("  test", choc::text::trimEnd(std::string("  test  ")));
		CHOC_EXPECT_EQ("", choc::text::trimEnd(std::string("  ")));
		CHOC_EXPECT_EQ("", choc::text::trimEnd(std::string(" ")));
		CHOC_EXPECT_EQ("", choc::text::trimEnd(std::string("")));
	}

	{
		CHOC_TEST(EndsWith)

		CHOC_EXPECT_TRUE(choc::text::endsWith("test", "t"));
		CHOC_EXPECT_TRUE(choc::text::endsWith("test", "st"));
		CHOC_EXPECT_TRUE(choc::text::endsWith("test", "est"));
		CHOC_EXPECT_TRUE(choc::text::endsWith("test", "test"));
		CHOC_EXPECT_FALSE(choc::text::endsWith("test", "x"));
		CHOC_EXPECT_FALSE(choc::text::endsWith("test", "ttest"));
		CHOC_EXPECT_TRUE(choc::text::endsWith("test", ""));
	}

	{
		CHOC_TEST(Durations)

		CHOC_EXPECT_EQ("0 sec", choc::text::getDurationDescription(std::chrono::milliseconds(0)));
		CHOC_EXPECT_EQ("999 microseconds", choc::text::getDurationDescription(std::chrono::microseconds(999)));
		CHOC_EXPECT_EQ("1 microsecond", choc::text::getDurationDescription(std::chrono::microseconds(1)));
		CHOC_EXPECT_EQ("-1 microsecond", choc::text::getDurationDescription(std::chrono::microseconds(-1)));
		CHOC_EXPECT_EQ("1 ms", choc::text::getDurationDescription(std::chrono::milliseconds(1)));
		CHOC_EXPECT_EQ("-1 ms", choc::text::getDurationDescription(std::chrono::milliseconds(-1)));
		CHOC_EXPECT_EQ("2 ms", choc::text::getDurationDescription(std::chrono::milliseconds(2)));
		CHOC_EXPECT_EQ("1.5 ms", choc::text::getDurationDescription(std::chrono::microseconds(1495)));
		CHOC_EXPECT_EQ("2 ms", choc::text::getDurationDescription(std::chrono::microseconds(1995)));
		CHOC_EXPECT_EQ("1 sec", choc::text::getDurationDescription(std::chrono::seconds(1)));
		CHOC_EXPECT_EQ("2 sec", choc::text::getDurationDescription(std::chrono::seconds(2)));
		CHOC_EXPECT_EQ("2.3 sec", choc::text::getDurationDescription(std::chrono::milliseconds(2300)));
		CHOC_EXPECT_EQ("2.31 sec", choc::text::getDurationDescription(std::chrono::milliseconds(2310)));
		CHOC_EXPECT_EQ("2.31 sec", choc::text::getDurationDescription(std::chrono::milliseconds(2314)));
		CHOC_EXPECT_EQ("2.31 sec", choc::text::getDurationDescription(std::chrono::milliseconds(2305)));
		CHOC_EXPECT_EQ("1 min 3 sec", choc::text::getDurationDescription(std::chrono::milliseconds(63100)));
		CHOC_EXPECT_EQ("2 min 3 sec", choc::text::getDurationDescription(std::chrono::milliseconds(123100)));
		CHOC_EXPECT_EQ("1 hour 2 min", choc::text::getDurationDescription(std::chrono::seconds(3726)));
		CHOC_EXPECT_EQ("-1 hour 2 min", choc::text::getDurationDescription(std::chrono::seconds(-3726)));
	}

	{
		CHOC_TEST(BytesSizes)

		CHOC_EXPECT_EQ("0 bytes", choc::text::getByteSizeDescription(0));
		CHOC_EXPECT_EQ("1 byte", choc::text::getByteSizeDescription(1));
		CHOC_EXPECT_EQ("2 bytes", choc::text::getByteSizeDescription(2));
		CHOC_EXPECT_EQ("1 KB", choc::text::getByteSizeDescription(1024));
		CHOC_EXPECT_EQ("1.1 KB", choc::text::getByteSizeDescription(1024 + 100));
		CHOC_EXPECT_EQ("1 MB", choc::text::getByteSizeDescription(1024 * 1024));
		CHOC_EXPECT_EQ("1.2 MB", choc::text::getByteSizeDescription((1024 + 200) * 1024));
		CHOC_EXPECT_EQ("1 GB", choc::text::getByteSizeDescription(1024 * 1024 * 1024));
		CHOC_EXPECT_EQ("1.3 GB", choc::text::getByteSizeDescription((1024 + 300) * 1024 * 1024));
	}

	{
		CHOC_TEST(UTF8)

		auto                    text = "line1\xd7\x90\n\xcf\x88line2\nli\xe1\xb4\x81ne3\nline4\xe1\xb4\xa8";
		choc::text::UTF8Pointer p(text);

		CHOC_EXPECT_TRUE(choc::text::findInvalidUTF8Data(text, std::string_view(text).length()) == nullptr);
		CHOC_EXPECT_EQ(2u, choc::text::findLineAndColumn(p, p.find("ine2")).line);
		CHOC_EXPECT_EQ(3u, choc::text::findLineAndColumn(p, p.find("ine2")).column);
		CHOC_EXPECT_TRUE(p.find("ine4").findStartOfLine(p).startsWith("line4"));

		CHOC_EXPECT_EQ(0x12345u, choc::text::createUnicodeFromHighAndLowSurrogates(choc::text::splitCodePointIntoSurrogatePair(0x12345u)));
	}

	{
		CHOC_TEST(TextTable)

		choc::text::TextTable table;
		table << "1" << "234" << "5";
		table.newRow();
		table << "" << "2345" << "x" << "y";
		table.newRow();
		table << "2345";

		CHOC_EXPECT_EQ(table.getNumRows(), 3u);
		CHOC_EXPECT_EQ(table.getNumColumns(), 4u);
		CHOC_EXPECT_EQ(table.toString("<", ";", ">"), std::string("<1   ;234 ;5; ><    ;2345;x;y><2345;    ; ; >"));
	}

	{
		CHOC_TEST(URIEncoding)
		CHOC_EXPECT_EQ(choc::text::percentEncodeURI("ABC://``\\123-abc~-xyz"), "ABC%3a%2f%2f%60%60%5c123-abc~-xyz");
	}

	{
		CHOC_TEST(SafeFilename)
		CHOC_EXPECT_EQ(choc::file::makeSafeFilename(""), "_");
		CHOC_EXPECT_EQ(choc::file::makeSafeFilename("//"), "_");
		CHOC_EXPECT_EQ(choc::file::makeSafeFilename("::sadf/sdfds123 sdf.sdfs."), "sadfsdfds123 sdf.sdfs.");
		CHOC_EXPECT_EQ(choc::file::makeSafeFilename("::,;sadf/sdfds123 sdfsd.xyz", 10), "sadfsd.xyz");
		CHOC_EXPECT_EQ(choc::file::makeSafeFilename("\\sa'df/sdfds123 sdfsd.xyzdfgdfgdfg", 10), "sa.xyzdfgdfgdfg");
	}
}

inline void testFileUtilities(choc::test::TestProgress &progress)
{
	CHOC_CATEGORY(Files);

	{
		CHOC_TEST(glob)
		choc::text::glob p1("*.xyz;*.foo", false), p2("*", false), p3, p4("abc?.x", true);

		CHOC_EXPECT_TRUE(p1.matches("sdf.xyz") && p1.matches("sdf.XyZ") && p1.matches(".xyz") && p1.matches("dfg.foo"));
		CHOC_EXPECT_FALSE(p1.matches("sdf.xxyz") || p1.matches("") || p1.matches("abc.xy") || p1.matches(".xyzz"));
		CHOC_EXPECT_TRUE(p2.matches("") && p2.matches("abcd"));
		CHOC_EXPECT_TRUE(p3.matches("") && p3.matches("dfgdfg"));
		CHOC_EXPECT_TRUE(p4.matches("abcd.x"));
		CHOC_EXPECT_FALSE(p4.matches("abcd.X") || p4.matches("abcdd.x") || p4.matches("abc.x"));
	}
}

inline void testFileWatcher(choc::test::TestProgress &progress)
{
	CHOC_CATEGORY(FileWatcher);

	try
	{
		CHOC_TEST(Watch)

		choc::file::TempFile tempFolder("choc_test_tmp");
		auto                 folder   = tempFolder.file / choc::file::TempFile::createRandomFilename("choc_test", {});
		auto                 testFile = folder / "blah" / "test1.txt";
		choc::file::replaceFileWithContent(testFile, "blah");
		std::filesystem::remove_all(folder / "blah");
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		std::mutex  lock;
		std::string lastEvent;

		choc::file::Watcher watcher(folder, [&](const choc::file::Watcher::Event &e) {
			std::scoped_lock lg(lock);

			switch (e.eventType)
			{
				case choc::file::Watcher::EventType::modified:
					lastEvent += " modified";
					break;
				case choc::file::Watcher::EventType::created:
					lastEvent += " created";
					break;
				case choc::file::Watcher::EventType::destroyed:
					lastEvent += " destroyed";
					break;
				case choc::file::Watcher::EventType::renamed:
					lastEvent += " renamed";
					break;
				case choc::file::Watcher::EventType::ownerChanged:
					lastEvent += " ownerChanged";
					break;
				case choc::file::Watcher::EventType::other:
				default:
					lastEvent += " other";
					break;
			}

			if (e.fileType == choc::file::Watcher::FileType::file)
				lastEvent += " file ";
			if (e.fileType == choc::file::Watcher::FileType::folder)
				lastEvent += " folder ";

			lastEvent += e.file.filename().string();
		});

		auto waitFor = [&](std::string_view contentNeeded) {
			for (int i = 0; i < 400; ++i)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				std::scoped_lock lg(lock);

				if (choc::text::contains(lastEvent, contentNeeded))
					return;
			}

			std::scoped_lock lg(lock);
			CHOC_FAIL("Expected '" + std::string(contentNeeded) + "' in '" + lastEvent + "'");
		};

		choc::file::replaceFileWithContent(testFile, "blah");
		waitFor("created folder blah");
#ifdef __MINGW32__        // for some reason the mingw runner seems to have low-res timestamps
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
#endif
		choc::file::replaceFileWithContent(testFile, "blah2");
		waitFor("modified file test1.txt");
		std::filesystem::remove_all(testFile);
		waitFor("destroyed file test1.txt");
	}
	CHOC_CATCH_UNEXPECTED_EXCEPTION
}

inline void testTimers(choc::test::TestProgress &progress)
{
	CHOC_CATEGORY(MessageLoop);

	{
		CHOC_TEST(Timers)

		int                      count = 0, messageCount = 0;
		bool                     messageThread1 = false, messageThread2 = false;
		choc::messageloop::Timer t1, t2;

		runTestOnMessageThread([&](const std::function<void()> &finished) {
            t1 = choc::messageloop::Timer (100, [&]
            {
                return ++count != 13;
            });

            t2 = choc::messageloop::Timer (1500, [&]
            {
                if (count < 13)
                    return true;

                choc::messageloop::postMessage ([&finished, &messageCount, count]
                {
                    messageCount = count;
                    finished();
                });

                return false;
            });

            choc::messageloop::postMessage ([&] { messageThread1 = choc::messageloop::callerIsOnMessageThread(); });
            auto t = std::thread ([&] { messageThread2 = ! choc::messageloop::callerIsOnMessageThread(); });
            t.join(); },
		                       [&] {
			                       t1 = {};
			                       t2 = {};
		                       });

		CHOC_EXPECT_EQ(messageCount, 13);
		CHOC_EXPECT_TRUE(messageThread1);
		CHOC_EXPECT_TRUE(messageThread2);
	}
}
//==============================================================================
inline bool runAllTests(choc::test::TestProgress &progress, bool multithread)
{
	choc::threading::TaskThread emergencyKillThread;
	int                         secondsElapsed = 0;

	emergencyKillThread.start(1000, [&] {
		if (++secondsElapsed > 300)
		{
			std::cerr << "FAIL!! Tests timed out and were killed!" << std::endl;
			std::terminate();
		}

		return true;
	});

	choc::messageloop::initialise();

	std::vector<std::function<void(choc::test::TestProgress &)>> testFunctions{
	    testFileWatcher,
	    testStringUtilities,
	    testFileUtilities,
	    testTimers};

	auto t = std::thread([&] {
		if (multithread)
		{
			std::vector<std::future<void>> futures;
			std::mutex                     progressLock;

			for (auto &fn : testFunctions)
			{
				futures.emplace_back(std::async(std::launch::async, [fn, &progress, &progressLock] {
					std::ostringstream       testOutput;
					choc::test::TestProgress p;
					p.printMessage = [&](std::string_view m) { testOutput << m << "\n"; };
					fn(p);

					std::scoped_lock lock(progressLock);

					progress.print(choc::text::trimEnd(testOutput.str()));
					progress.numPasses += p.numPasses;
					progress.numFails += p.numFails;

					for (auto &failed : p.failedTests)
						progress.failedTests.push_back(failed);
				}));
			}

			for (auto &f : futures)
				f.wait();
		}
		else
		{
			for (auto &fn : testFunctions)
				fn(progress);
		}

		choc::messageloop::stop();
	});

	choc::messageloop::run();
	t.join();

	progress.printReport();
	return progress.numFails == 0;
}

}        // namespace choc_unit_tests

#endif
