#include "TerminalBuffer.h"
#include "ANSIParser.h"
#include "SSHConnection.h"
#include "SSHChannel.h"
#include "ProfileStorage.h"
#include <QTest>
#include <QElapsedTimer>
#include <QVector>
#include <QDebug>

class TestPerformance : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Performance benchmarks
    void benchmarkTerminalBufferAppend();
    void benchmarkTerminalBufferScrollback();
    void benchmarkANSIParserSimple();
    void benchmarkANSIParserComplex();
    void benchmarkProfileSerialization();
    void benchmarkConnectionSetup();
    void benchmarkChannelThroughput();
    void benchmarkMemoryUsage();

private:
    QString generateLargeOutput(int lines, int charsPerLine);
    QString generateComplexANSI(int count);
};

void TestPerformance::initTestCase()
{
    qInfo() << "Starting performance benchmarks";
}

void TestPerformance::cleanupTestCase()
{
    qInfo() << "Performance benchmarks complete";
}

QString TestPerformance::generateLargeOutput(int lines, int charsPerLine)
{
    QString result;
    result.reserve(lines * (charsPerLine + 1));

    for (int i = 0; i < lines; ++i) {
        for (int j = 0; j < charsPerLine; ++j) {
            result.append(QChar('A' + (j % 26)));
        }
        result.append('\n');
    }

    return result;
}

QString TestPerformance::generateComplexANSI(int count)
{
    QString result;
    result.reserve(count * 50); // Approximate

    for (int i = 0; i < count; ++i) {
        // Mix of cursor movements, colors, and formatting
        result.append("\033[H");                  // Home
        result.append("\033[2J");                 // Clear screen
        result.append("\033[1;31m");              // Bold red
        result.append("Line ").append(QString::number(i));
        result.append("\033[0m");                 // Reset
        result.append("\033[10;20H");             // Position
        result.append("\033[32;44m");             // Green on blue
        result.append("Test");
        result.append("\033[0m\n");
    }

    return result;
}

void TestPerformance::benchmarkTerminalBufferAppend()
{
    TerminalBuffer buffer;
    const int numLines = 10000;
    const int charsPerLine = 80;

    QString testData = generateLargeOutput(numLines, charsPerLine);
    QStringList lines = testData.split('\n', Qt::SkipEmptyParts);

    QElapsedTimer timer;
    timer.start();

    QBENCHMARK {
        TerminalBuffer testBuffer;
        for (const QString& line : lines) {
            testBuffer.appendLine(line);
        }
    }

    qint64 elapsed = timer.elapsed();
    qInfo() << "TerminalBuffer append:" << numLines << "lines in" << elapsed << "ms";
    qInfo() << "  Throughput:" << (numLines * 1000.0 / elapsed) << "lines/sec";
}

void TestPerformance::benchmarkTerminalBufferScrollback()
{
    const int maxScrollback = 100000;
    const int charsPerLine = 80;

    TerminalBuffer buffer;
    buffer.setMaxScrollback(maxScrollback);

    // Fill buffer
    QString testLine(charsPerLine, 'X');
    for (int i = 0; i < maxScrollback; ++i) {
        buffer.appendLine(testLine);
    }

    QElapsedTimer timer;
    timer.start();

    QBENCHMARK {
        // Access scrollback
        for (int i = 0; i < 1000; ++i) {
            QString line = buffer.getLine(i % maxScrollback);
        }
    }

    qint64 elapsed = timer.elapsed();
    qInfo() << "TerminalBuffer scrollback access: 1000 random reads in" << elapsed << "ms";
}

void TestPerformance::benchmarkANSIParserSimple()
{
    ANSIParser parser;
    const int numLines = 1000;
    QString simpleText = generateLargeOutput(numLines, 80);

    QElapsedTimer timer;
    timer.start();

    QBENCHMARK {
        ANSIParser testParser;
        testParser.parseToTokens(simpleText);
    }

    qint64 elapsed = timer.elapsed();
    qInfo() << "ANSI Parser (simple):" << numLines << "lines in" << elapsed << "ms";
}

void TestPerformance::benchmarkANSIParserComplex()
{
    ANSIParser parser;
    const int numSequences = 1000;
    QString complexText = generateComplexANSI(numSequences);

    QElapsedTimer timer;
    timer.start();

    QBENCHMARK {
        ANSIParser testParser;
        testParser.parseToTokens(complexText);
    }

    qint64 elapsed = timer.elapsed();
    qInfo() << "ANSI Parser (complex):" << numSequences << "sequences in" << elapsed << "ms";
    qInfo() << "  Throughput:" << (numSequences * 1000.0 / elapsed) << "sequences/sec";
}

void TestPerformance::benchmarkProfileSerialization()
{
    const int numProfiles = 100;
    QVector<ConnectionProfile> profiles;

    // Create test profiles
    for (int i = 0; i < numProfiles; ++i) {
        ConnectionProfile profile(QString("test%1").arg(i),
                                  QString("host%1.example.com").arg(i),
                                  22 + i,
                                  QString("user%1").arg(i));
        profiles.append(profile);
    }

    ProfileStorage storage;
    QElapsedTimer timer;

    // Benchmark saving
    timer.start();
    QBENCHMARK {
        for (const auto& profile : profiles) {
            storage.saveProfile(profile);
        }
    }
    qint64 saveTime = timer.elapsed();

    // Benchmark loading
    timer.restart();
    QBENCHMARK {
        QVector<ConnectionProfile> loaded = storage.loadAllProfiles();
    }
    qint64 loadTime = timer.elapsed();

    qInfo() << "Profile serialization:";
    qInfo() << "  Save:" << numProfiles << "profiles in" << saveTime << "ms";
    qInfo() << "  Load:" << numProfiles << "profiles in" << loadTime << "ms";
}

void TestPerformance::benchmarkConnectionSetup()
{
    // This benchmark measures connection setup overhead (without actual connection)
    const int iterations = 100;

    QElapsedTimer timer;
    timer.start();

    QBENCHMARK {
        for (int i = 0; i < iterations; ++i) {
            ConnectionProfile profile("test", "example.com", 22, "user");
            SSHConnection connection(profile);
            QVERIFY(connection.initializeSession());
            QVERIFY(connection.setSessionOptions());
        }
    }

    qint64 elapsed = timer.elapsed();
    qInfo() << "Connection object creation:" << iterations << "iterations in" << elapsed << "ms";
    qInfo() << "  Average:" << (elapsed * 1.0 / iterations) << "ms per connection";
}

void TestPerformance::benchmarkChannelThroughput()
{
    // Benchmark theoretical maximum throughput for channel operations
    const int dataSize = 1024 * 1024; // 1MB
    QByteArray testData(dataSize, 'X');

    QElapsedTimer timer;
    timer.start();

    QBENCHMARK {
        // Simulate chunked data processing
        int bytesProcessed = 0;
        const int chunkSize = 4096;

        while (bytesProcessed < dataSize) {
            int toProcess = qMin(chunkSize, dataSize - bytesProcessed);
            QByteArray chunk = testData.mid(bytesProcessed, toProcess);
            bytesProcessed += toProcess;
        }
    }

    qint64 elapsed = timer.elapsed();
    double throughput = (dataSize / 1024.0 / 1024.0) / (elapsed / 1000.0); // MB/s
    qInfo() << "Channel data processing:" << dataSize << "bytes in" << elapsed << "ms";
    qInfo() << "  Throughput:" << throughput << "MB/s";
}

void TestPerformance::benchmarkMemoryUsage()
{
    qInfo() << "Memory usage benchmarks:";

    // Test 1: TerminalBuffer memory usage
    {
        TerminalBuffer buffer;
        buffer.setMaxScrollback(100000);

        QString testLine(80, 'X');
        for (int i = 0; i < 100000; ++i) {
            buffer.appendLine(testLine);
        }

        // Approximate memory usage: 100,000 lines * 80 chars * 2 bytes (QString)
        qint64 approxMemory = 100000 * 80 * 2;
        qInfo() << "  TerminalBuffer (100k lines):" << (approxMemory / 1024 / 1024) << "MB (approx)";
    }

    // Test 2: ConnectionProfile objects
    {
        QVector<ConnectionProfile> profiles;
        profiles.reserve(1000);

        for (int i = 0; i < 1000; ++i) {
            profiles.append(ConnectionProfile(QString("test%1").arg(i),
                                              QString("host%1.example.com").arg(i),
                                              22,
                                              QString("user%1").arg(i)));
        }

        // Rough estimate: ~200 bytes per profile
        qint64 approxMemory = 1000 * 200;
        qInfo() << "  1000 ConnectionProfiles:" << (approxMemory / 1024) << "KB (approx)";
    }

    // Test 3: ANSI parsing overhead
    {
        ANSIParser parser;
        QString complexText = generateComplexANSI(10000);

        auto tokens = parser.parseToTokens(complexText);

        // Estimate: input size + token overhead
        qint64 inputSize = complexText.size() * 2; // QString overhead
        qint64 tokenSize = tokens.size() * 32;      // Approximate token size
        qInfo() << "  ANSI Parser (10k sequences):" << ((inputSize + tokenSize) / 1024) << "KB (approx)";
    }
}

QTEST_MAIN(TestPerformance)
#include "test_performance.moc"
