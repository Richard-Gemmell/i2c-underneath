// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_I2C_TIMING_ANALYSER_TEST_H
#define I2C_UNDERNEATH_I2C_TIMING_ANALYSER_TEST_H
#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include <analysis/i2c_timing_analyser.h>
#include <analysis/i2c_timing_analysis.h>

namespace analysis {
class I2CTimingAnalyserTest : public TestSuite {
private:
    static common::hal::FakeClock clock;
    static const uint32_t nanos_per_tick = common::hal::FakeClock::nanos_per_tick;
    const static size_t MAX_EVENTS = 1024;
    const static bus_trace::BusEventFlags SDA_LINE_CHANGED = bus_trace::BusEventFlags::SDA_LINE_CHANGED;
    const static bus_trace::BusEventFlags SCL_LINE_CHANGED = bus_trace::BusEventFlags::SCL_LINE_CHANGED;
    const static bus_trace::BusEventFlags SDA_LINE_STATE = bus_trace::BusEventFlags::SDA_LINE_STATE;
    const static bus_trace::BusEventFlags SCL_LINE_STATE = bus_trace::BusEventFlags::SCL_LINE_STATE;

    // These 2 byte values contain all four bit transitions 0->0, 0->1, 1->1 and 1->0
    // The first and last bit of each byte affect the edges for ACKs
    // so one byte is the inverse of the other.
    const static uint8_t BYTE_A = 0x58; // 0101 1000
    const static uint8_t BYTE_B = 0xA7; // 1010 0111
    const static uint8_t ADDRESS = 0x53;// 0101 0011

public:
    void setUp() override {
        TestSuite::setUp();
        clock.reset();
    }

    static void log_value(const char* msg, const analysis::DurationStatistics& actual) {
        Serial.printf("%s ", msg);
        Serial.println(actual);
    }

    static void given_a_valid_trace(bus_trace::BusTrace& trace) {
        // BUS is idle to start with
        trace.add_event(0, SDA_LINE_STATE | SCL_LINE_STATE);
        // START
        const uint32_t tf = 110/nanos_per_tick; // Fall time
        trace.add_event(tf, SDA_LINE_CHANGED | SCL_LINE_STATE);
        uint32_t tHD_STA = (4'000 + 120)/nanos_per_tick;
        trace.add_event(tHD_STA, SCL_LINE_CHANGED);

        uint32_t tLOW = 4'700/nanos_per_tick;
        uint32_t tHIGH = 4'000/nanos_per_tick;
        uint32_t tHD_DAT = 2'000/nanos_per_tick;
        // 7 bit Address - 101 0011
        // 1 (0->1)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(tLOW+4, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(tHIGH+5, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 0 (1->0)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED);
        trace.add_event(tLOW+7, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace.add_event(tHIGH+8, SCL_LINE_CHANGED);
        // 1 (0->1)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(tLOW+10, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(tHIGH+11, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 0 (1->0)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED);
        trace.add_event(tLOW+13, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace.add_event(tHIGH+14, SCL_LINE_CHANGED);
        // 0 (0->0)
        trace.add_event(tLOW+15, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace.add_event(tHIGH+16, SCL_LINE_CHANGED);
        // 1 (0->1)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(tLOW+18, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(tHIGH+19, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 1 (1->1)
        trace.add_event(tLOW+20, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(tHIGH+21, SCL_LINE_CHANGED | SDA_LINE_STATE);

        // READ/WRITE Bit
        // 1 (1->1)
        trace.add_event(tLOW+22, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(tHIGH+23, SCL_LINE_CHANGED | SDA_LINE_STATE);

        // ACK address - by slave
        // 0 (1->0)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED);
        trace.add_event(tLOW+25, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace.add_event(tHIGH+26, SCL_LINE_CHANGED);

        // Data Byte - 0101 1000
        // 0 (0->0)
        trace.add_event(tLOW+27, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace.add_event(tHIGH+28, SCL_LINE_CHANGED);
        // 1 (0->1)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(tLOW+30, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(tHIGH+31, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 0 (1->0)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED);
        trace.add_event(tLOW+33, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace.add_event(tHIGH+34, SCL_LINE_CHANGED);
        // 1 (0->1)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(tLOW+36, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(tHIGH+37, SCL_LINE_CHANGED | SDA_LINE_STATE);

        // 1 (1->1)
        trace.add_event(tLOW+38, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(tHIGH+39, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 0 (1->0)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED);
        trace.add_event(tLOW+41, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace.add_event(tHIGH+42, SCL_LINE_CHANGED);
        // 0 (0->0)
        trace.add_event(tLOW+43, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace.add_event(tHIGH+44, SCL_LINE_CHANGED);
        // 0 (0->0)
        trace.add_event(tLOW+45, SCL_LINE_CHANGED | SCL_LINE_STATE);
        trace.add_event(tHIGH+46, SCL_LINE_CHANGED);

        // ACK data byte - by master
        // 0 (0->0)
//        trace.add_event(tLOW, SCL_LINE_CHANGED | SCL_LINE_STATE);
//        trace.add_event(tHIGH, SCL_LINE_CHANGED);

        // Data Byte
        // NACK data byte - master NACKS final byte to say it's done
        // 1 (0->1)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        trace.add_event(tLOW+48, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        trace.add_event(tHIGH+49, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // STOP (1->0)
        trace.add_event(tHD_DAT, SDA_LINE_CHANGED);
        trace.add_event(tLOW+51, SCL_LINE_CHANGED | SCL_LINE_STATE);
        uint32_t tSU_STO = 4'000/nanos_per_tick;
        trace.add_event(tSU_STO+1, SDA_LINE_CHANGED | SDA_LINE_STATE | SCL_LINE_STATE);
    }

    static bool trace_matches_expected(const bus_trace::BusTrace& trace) {
        // Create an I2C message. The master reads 2 bytes from the slave.
        bus_trace::BusTrace expected_trace(MAX_EVENTS);
        bus_trace::BusTraceBuilder builder(expected_trace, bus_trace::BusTraceBuilder::TimingStrategy::Min, common::i2c_specification::StandardMode);
        builder.bus_initially_idle().start_bit()
                .address_byte(ADDRESS, bus_trace::BusTraceBuilder::READ).ack()
                .data_byte(BYTE_A)//.ack()
//                .data_byte(BYTE_B).nack()
                .nack()
                .stop_bit();

        // Check that 'trace' contains the edges we expect.
        // Ignore the timings
        return expected_trace.compare_edges(trace) == SIZE_MAX;
    }

    static void test_trace_is_valid() {
        // GIVEN we've built a test trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // THEN the test trace represents the expected I2C message
        TEST_ASSERT_TRUE(trace_matches_expected(trace))
    }

    static void analysis_records_start_hold_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto analysis = I2CTimingAnalyser::analyse(trace);

        // THEN the start hold time is analysed correctly
        TEST_ASSERT_EQUAL_UINT32(1, analysis.start_hold_time.count());
        TEST_ASSERT_EQUAL_UINT32(4'120, analysis.start_hold_time.average());
    }

    static void analysis_records_clock_high_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace).scl_high_time;

        // THEN the clock high time is measured correctly
//        log_value("Clock HIGH", actual);
        TEST_ASSERT_EQUAL_UINT32(18, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'010, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'098, actual.max());
    }

    static void analysis_records_clock_low_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace).scl_low_time;

        // THEN the clock low time is measured correctly
//        log_value("Clock LOW", actual);
        TEST_ASSERT_EQUAL_UINT32(19, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'730, actual.min());
        TEST_ASSERT_EQUAL_UINT32(6'802, actual.max());
    }

    static void analysis_records_clock_frequency() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace).clock_frequency;

        // THEN the SCL clock frequency is measured correctly
//        log_value("SCL clock frequency", actual);
        TEST_ASSERT_EQUAL_UINT32(18, actual.count());
        TEST_ASSERT_EQUAL_UINT32(91'793, actual.min());
        TEST_ASSERT_EQUAL_UINT32(114'129, actual.max());
    }

    static void analysis_records_setup_stop_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace).stop_setup_time;

        // THEN the stop setup time (tSU;STO) is recorded correctly
//        log_value("Setup stop time", actual);
        TEST_ASSERT_EQUAL_UINT32(1, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'002, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'002, actual.max());
    }

    void test() final {
        RUN_TEST(test_trace_is_valid);
        RUN_TEST(analysis_records_start_hold_time);
        // TODO: Test SCL clock for traces of broken I2C transfer
        RUN_TEST(analysis_records_clock_high_time);
        RUN_TEST(analysis_records_clock_low_time);
        RUN_TEST(analysis_records_clock_frequency);
        RUN_TEST(analysis_records_setup_stop_time);
    }

    I2CTimingAnalyserTest() : TestSuite(__FILE__) {};
};
common::hal::FakeClock I2CTimingAnalyserTest::clock;
} // analysis
#endif //I2C_UNDERNEATH_I2C_TIMING_ANALYSER_TEST_H
