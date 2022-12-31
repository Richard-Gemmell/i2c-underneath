// Copyright (c) 2022 Richard Gemmell
// Released under the MIT License. See license.txt. (https://opensource.org/licenses/MIT)

#ifndef I2C_UNDERNEATH_I2C_TIMING_ANALYSER_TEST_H
#define I2C_UNDERNEATH_I2C_TIMING_ANALYSER_TEST_H
#include <unity.h>
#include <Arduino.h>
#include "utils/test_suite.h"
#include "fakes/common/hal/fake_clock.h"
#include "bus_trace/bus_trace_builder.h"
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

    constexpr static uint32_t tLOW = 4'700/nanos_per_tick;
    constexpr static uint32_t tHIGH = 4'000/nanos_per_tick;
    constexpr static uint32_t tHD_DAT = 2'000/nanos_per_tick;

    const static uint16_t SDA_RISE = 500;
    const static uint16_t SCL_RISE = 1'000;
    const static uint16_t SDA_FALL = 150;
    const static uint16_t SCL_FALL = 300;

public:
    void setUp() override {
        TestSuite::setUp();
        clock.reset();
    }

    static void log_value(const char* msg, const analysis::DurationStatistics& actual) {
//        Serial.printf("%s ", msg);
//        Serial.println(actual);
    }

    static void add_event(bus_trace::BusTrace& trace, uint32_t delta_t_in_ticks, bus_trace::BusEventFlags flags) {
        trace.add_event(bus_trace::BusEvent(delta_t_in_ticks, flags));
    }

    static void add_start(bus_trace::BusTrace& trace) {
        // Adds a start bit
        const uint32_t tBUF = 4'846 / nanos_per_tick; // Bus Free Time
        add_event(trace, tBUF, SDA_LINE_CHANGED | SCL_LINE_STATE);
        uint32_t tHD_STA = (4'000 + 120) / nanos_per_tick;
        add_event(trace, tHD_STA, SCL_LINE_CHANGED);
    }

    static void add_address_byte(bus_trace::BusTrace& trace) {
        // 7 bit Address - 101 0011
        // 1 (0->1)
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        add_event(trace, tLOW+4, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        add_event(trace, tHIGH+5, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 0 (1->0)
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED);
        add_event(trace, tLOW+7, SCL_LINE_CHANGED | SCL_LINE_STATE);
        add_event(trace, tHIGH+8, SCL_LINE_CHANGED);
        // 1 (0->1)
        add_event(trace, tHD_DAT-3, SDA_LINE_CHANGED | SDA_LINE_STATE);
        add_event(trace, tLOW+10, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        add_event(trace, tHIGH+11, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 0 (1->0)
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED);
        add_event(trace, tLOW+13, SCL_LINE_CHANGED | SCL_LINE_STATE);
        add_event(trace, tHIGH+14, SCL_LINE_CHANGED);
        // 0 (0->0)
        add_event(trace, tLOW+15, SCL_LINE_CHANGED | SCL_LINE_STATE);
        add_event(trace, tHIGH+16, SCL_LINE_CHANGED);
        // 1 (0->1)
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        add_event(trace, tLOW+18, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        add_event(trace, tHIGH+19, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 1 (1->1)
        add_event(trace, tLOW+20, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        add_event(trace, tHIGH+21, SCL_LINE_CHANGED | SDA_LINE_STATE);

        // READ/WRITE Bit
        // 1 (1->1)
        add_event(trace, tLOW+22, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        add_event(trace, tHIGH+23, SCL_LINE_CHANGED | SDA_LINE_STATE);

        // ACK address - by slave
        // 0 (1->0)
        add_event(trace, tHD_DAT+1, SDA_LINE_CHANGED);
        add_event(trace, tLOW+25, SCL_LINE_CHANGED | SCL_LINE_STATE);
        add_event(trace, tHIGH+26, SCL_LINE_CHANGED);
    }

    static void add_data_byte(bus_trace::BusTrace& trace) {
        // Include a short SDA spike. These are common when the transmitter releases SDA
        // before the receiver sends ACK
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED);
        add_event(trace, 20, SDA_LINE_CHANGED | SDA_LINE_STATE);

        // Data Byte - 0101 1000
        // 0 (0->0)
        add_event(trace, tLOW+27, SCL_LINE_CHANGED | SCL_LINE_STATE);
        add_event(trace, tHIGH+28, SCL_LINE_CHANGED);
        // 1 (0->1)
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        add_event(trace, tLOW+30, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        add_event(trace, tHIGH+31, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 0 (1->0)
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED);
        add_event(trace, tLOW+33, SCL_LINE_CHANGED | SCL_LINE_STATE);
        add_event(trace, tHIGH+34, SCL_LINE_CHANGED);
        // 1 (0->1)
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        add_event(trace, tLOW+36, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        add_event(trace, tHIGH+37, SCL_LINE_CHANGED | SDA_LINE_STATE);

        // 1 (1->1)
        add_event(trace, tLOW+38, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        add_event(trace, tHIGH+39, SCL_LINE_CHANGED | SDA_LINE_STATE);
        // 0 (1->0)
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED);
        add_event(trace, tLOW+41, SCL_LINE_CHANGED | SCL_LINE_STATE);
        add_event(trace, tHIGH+42, SCL_LINE_CHANGED);
        // 0 (0->0)
        add_event(trace, tLOW+43, SCL_LINE_CHANGED | SCL_LINE_STATE);
        add_event(trace, tHIGH+44, SCL_LINE_CHANGED);
        // 0 (0->0)
        add_event(trace, tLOW+45, SCL_LINE_CHANGED | SCL_LINE_STATE);
        add_event(trace, tHIGH+46, SCL_LINE_CHANGED);

        // NACK data byte - master NACKS final byte to say it's done
        // 1 (0->1)
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED | SDA_LINE_STATE);
        add_event(trace, tLOW+48, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        add_event(trace, tHIGH+49, SCL_LINE_CHANGED | SDA_LINE_STATE);
    }

    static void add_stop(bus_trace::BusTrace& trace) {
        add_event(trace, tHD_DAT, SDA_LINE_CHANGED);
        add_event(trace, tLOW+51, SCL_LINE_CHANGED | SCL_LINE_STATE);
        uint32_t tSU_STO = 4'000/nanos_per_tick;
        add_event(trace, tSU_STO+1, SDA_LINE_CHANGED | SDA_LINE_STATE | SCL_LINE_STATE);
    }

    static void given_a_valid_trace(bus_trace::BusTrace& trace) {
        // BUS is idle to start with
        add_event(trace, 0, SDA_LINE_STATE | SCL_LINE_STATE);

        // START
        add_start(trace);

        // 7 bit Address - 101 0011
        // followed by READ/WRITE Bit
        // followed by ACK from slave
        add_address_byte(trace);

        // Data Byte - 0101 1000
        // followed by NACK from master
        add_data_byte(trace);

        // STOP (1->0)
        add_stop(trace);
    }

    static void given_2_messages_separated_by_a_repeated_start(bus_trace::BusTrace& trace) {
        // BUS is idle to start with
        add_event(trace, 0, SDA_LINE_STATE | SCL_LINE_STATE);

        // First Message
        add_start(trace);
        add_address_byte(trace);
        add_data_byte(trace);

        // Repeated Start
        // SCL rises as if we're about to write a 1.
        // Assume SDA is already high because the last byte was NACKed
        add_event(trace, tLOW+53, SCL_LINE_CHANGED | SCL_LINE_STATE | SDA_LINE_STATE);
        // SDA falls while SCL is still high
        uint32_t tSU_STA = 4'700/nanos_per_tick;
        add_event(trace, tSU_STA+1, SDA_LINE_CHANGED | SCL_LINE_STATE);

        // Second Message
        add_address_byte(trace);
        add_data_byte(trace);
        add_stop(trace);
    }

    static void given_2_messages_separated_by_stop(bus_trace::BusTrace& trace) {
        // BUS is idle to start with
        add_event(trace, 0, SDA_LINE_STATE | SCL_LINE_STATE);

        // First Message
        add_start(trace);
        add_address_byte(trace);
        add_data_byte(trace);
        add_stop(trace);

        // Second Message
        add_start(trace);
        add_address_byte(trace);
        add_data_byte(trace);
        add_stop(trace);
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
        TEST_ASSERT_TRUE(trace_matches_expected(trace.to_message()));
    }

    static void analysis_records_raw_start_hold_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto analysis = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0);

        // THEN the start hold time is analysed correctly
        TEST_ASSERT_EQUAL_UINT32(1, analysis.start_hold_time.count());
        TEST_ASSERT_EQUAL_UINT32(4'120, analysis.start_hold_time.average());
    }

    static void analysis_adjusts_start_hold_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto analysis = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE, SDA_FALL, SCL_FALL);

        // THEN the start hold time is analysed correctly
        TEST_ASSERT_EQUAL_UINT32(1, analysis.start_hold_time.count());
        TEST_ASSERT_EQUAL_UINT32(3'910, analysis.start_hold_time.average());
    }

    static void analysis_records_start_hold_between_messages() {
        // GIVEN 2 messages separated by a STOP
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_2_messages_separated_by_stop(trace);

        // WHEN we analyse the trace
        auto analysis = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0);

        // THEN we capture 2 start hold times; one for each message
        TEST_ASSERT_EQUAL_UINT32(2, analysis.start_hold_time.count());
        TEST_ASSERT_EQUAL_UINT32(4'120, analysis.start_hold_time.average());
    }

    static void analysis_records_raw_clock_high_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0).scl_high_time;

        // THEN the clock high time is measured correctly
        log_value("Clock HIGH", actual);
        TEST_ASSERT_EQUAL_UINT32(18, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'010, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'098, actual.max());
    }

    static void analysis_adjusts_clock_high_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE, SDA_FALL, SCL_FALL).scl_high_time;

        // THEN the clock high time is measured correctly
        log_value("Clock HIGH", actual);
        TEST_ASSERT_EQUAL_UINT32(4'010 - 723, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'098 - 723, actual.max());
    }

    static void analysis_records_raw_clock_low_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0).scl_low_time;

        // THEN the clock low time is measured correctly
        log_value("Clock LOW", actual);
        // WARNING: Extra LOW
        // Treat the period before SCL goes HIGH in the STOP bit as a LOW period.
        // The spec doesn't say whether it should or should not but doesn't give it another name.
        TEST_ASSERT_EQUAL_UINT32(19, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'730, actual.min());
        TEST_ASSERT_EQUAL_UINT32(6'802, actual.max());
    }

    static void analysis_adjusts_clock_low_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE, SDA_FALL, SCL_FALL).scl_low_time;

        // THEN the clock low time is measured correctly
        log_value("Clock LOW", actual);
        TEST_ASSERT_EQUAL_UINT32(4'730 - 578, actual.min());
        TEST_ASSERT_EQUAL_UINT32(6'802 - 578, actual.max());
    }

    static void analysis_records_clock_frequency() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE, SDA_FALL, SCL_FALL).clock_frequency;

        // THEN the SCL clock frequency is measured correctly
        log_value("SCL clock frequency", actual);
        TEST_ASSERT_EQUAL_UINT32(18, actual.count());
        TEST_ASSERT_EQUAL_UINT32(91'793, actual.min());
        TEST_ASSERT_EQUAL_UINT32(114'129, actual.max());
    }

    static void analysis_records_raw_setup_stop_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace with zero rise and fall times
        auto actual = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0).stop_setup_time;

        // THEN the stop setup time (tSU;STO) is the recorded time
//        log_value("Setup stop time", actual);
        TEST_ASSERT_EQUAL_UINT32(1, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'002, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'002, actual.max());
    }

    static void analysis_adjusts_setup_stop_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace with actual rise times
        auto actual = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE).stop_setup_time;

        // THEN the stop setup time (tSU;STO) is modified to account for the Teensy's trigger voltage
//        log_value("Setup stop time", actual);
        TEST_ASSERT_EQUAL_UINT32(1, actual.count());
        TEST_ASSERT_EQUAL_UINT32(3'200, actual.min());
        TEST_ASSERT_EQUAL_UINT32(3'200, actual.max());
    }

    static void analysis_records_raw_setup_start_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_2_messages_separated_by_a_repeated_start(trace);

        // WHEN we analyse the trace with zero rise and fall times
        auto actual = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0).start_setup_time;

        // THEN the repeated start setup time (tSU;STA) is the recorded time
        log_value("Setup time for repeated start", actual);
        TEST_ASSERT_EQUAL_UINT32(1, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'702, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'702, actual.max());
    }

    static void analysis_adjusts_setup_start_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_2_messages_separated_by_a_repeated_start(trace);

        // WHEN we analyse the trace with actual rise times
        auto actual = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE, SDA_FALL, SCL_FALL).start_setup_time;

        // THEN the repeated start setup time (tSU;STA) is the recorded time
        log_value("Setup time for repeated start", actual);
        TEST_ASSERT_EQUAL_UINT32(1, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'039, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'039, actual.max());
    }

    static void setup_start_is_not_set_after_stop() {
        // GIVEN a 2 messages separated by a STOP and START
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_2_messages_separated_by_stop(trace);

        // WHEN we analyse the trace
        auto actual = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0).start_setup_time;

        // THEN the repeated start setup time (tSU;STA) is not defined
        // as this isn't a repeated start
        log_value("Setup start", actual);
        TEST_ASSERT_EQUAL_UINT32(0, actual.count());
    }

    static void analysis_records_raw_bus_free_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_2_messages_separated_by_stop(trace);

        // WHEN we analyse the trace with zero rise and fall times
        auto actual = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0).bus_free_time;

        // THEN the bus free time (tBUF) is the recorded time
        log_value("Bus free time", actual);
        TEST_ASSERT_EQUAL_UINT32(1, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'846, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'846, actual.max());
    }

    static void analysis_adjusts_bus_free_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_2_messages_separated_by_stop(trace);

        // WHEN we analyse the trace with actual rise times
        auto actual = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE, SDA_FALL, SCL_FALL).bus_free_time;

        // THEN the bus free time (tBUF) is the recorded time
        log_value("Bus free time", actual);
        TEST_ASSERT_EQUAL_UINT32(4'846 - 302 - 60, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'846 - 302 - 60, actual.max());
    }

    static void analysis_records_raw_data_setup_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace with zero rise and fall times
        auto actual = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0).data_setup_time;

        // THEN the setup data time (tSU;DAT) is the recorded time
        log_value("Setup data time", actual);
        TEST_ASSERT_EQUAL_UINT32(13, actual.count());
        TEST_ASSERT_EQUAL_UINT32(4'708, actual.min());
        TEST_ASSERT_EQUAL_UINT32(4'802, actual.max());
    }

    static void analysis_adjusts_data_setup_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace with actual rise times
        auto actual = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE, SDA_FALL, SCL_FALL).data_setup_time;

        // THEN the setup data time (tSU;DAT) compensates for rise and fall times
        log_value("Setup data time", actual);
        TEST_ASSERT_EQUAL_UINT32(4'708 - 302 - 397, actual.min());  // SDA rose
        TEST_ASSERT_EQUAL_UINT32(4'802 - 91 - 397, actual.max());  // SDA fell
    }

    static void analysis_records_raw_data_hold_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);
        trace.printTo(Serial);

        // WHEN we analyse the trace with zero rise and fall times
        auto actual = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0).data_hold_time;

        // THEN the data hold time (tHD;DAT) is the recorded time
        log_value("Data hold time", actual);
        TEST_ASSERT_EQUAL_UINT32(13, actual.count());
        TEST_ASSERT_EQUAL_UINT32(1'994, actual.min());
        TEST_ASSERT_EQUAL_UINT32(2'002, actual.max());
    }

    static void analysis_adjusts_data_hold_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace with actual rise times
        auto actual = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE, SDA_FALL, SCL_FALL).data_hold_time;

        // THEN the data hold time (tHD;DAT) compensates for rise and fall times
        log_value("Setup data time", actual);
        TEST_ASSERT_EQUAL_UINT32(1'994 - 199 - 181, actual.min());  // SDA rose
        TEST_ASSERT_EQUAL_UINT32(2'002 - 60 - 181, actual.max());  // SDA fell
    }

    static void analysis_records_raw_data_valid_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace with zero rise and fall times
        auto actual = I2CTimingAnalyser::analyse(trace, 0, 0, 0, 0).data_valid_time;

        // THEN the data valid time (tVD;DAT) is the recorded time
        log_value("Data valid time", actual);
        TEST_ASSERT_EQUAL_UINT32(13, actual.count());
        TEST_ASSERT_EQUAL_UINT32(1'994, actual.min());
        TEST_ASSERT_EQUAL_UINT32(2'002, actual.max());
    }

    static void analysis_adjusts_data_valid_time() {
        // GIVEN a trace
        bus_trace::BusTrace trace(&clock, MAX_EVENTS);
        given_a_valid_trace(trace);

        // WHEN we analyse the trace with actual rise times
        auto analysis = I2CTimingAnalyser::analyse(trace, SDA_RISE, SCL_RISE, SDA_FALL, SCL_FALL);
        auto actual = analysis.data_valid_time;

        // THEN the data valid time (tVD;DAT) compensates for rise and fall times
        log_value("Setup valid time", actual);
        TEST_ASSERT_EQUAL_UINT32(1909, actual.min());
        TEST_ASSERT_EQUAL_UINT32(2120, actual.max());
    }

    void test() final {
        RUN_TEST(test_trace_is_valid);
        RUN_TEST(analysis_records_raw_start_hold_time);
        RUN_TEST(analysis_adjusts_start_hold_time);
        RUN_TEST(analysis_records_start_hold_between_messages);
//         TODO: Test SCL clock for traces of broken I2C transfer
        RUN_TEST(analysis_records_raw_clock_high_time);
        RUN_TEST(analysis_adjusts_clock_high_time);
        RUN_TEST(analysis_records_raw_clock_low_time);
        RUN_TEST(analysis_adjusts_clock_low_time);
        RUN_TEST(analysis_records_clock_frequency);
        RUN_TEST(analysis_records_raw_setup_stop_time);
        RUN_TEST(analysis_adjusts_setup_stop_time);
        RUN_TEST(analysis_records_raw_setup_start_time);
        RUN_TEST(analysis_adjusts_setup_start_time);
        RUN_TEST(setup_start_is_not_set_after_stop);
        RUN_TEST(analysis_records_raw_bus_free_time);
        RUN_TEST(analysis_adjusts_bus_free_time);
        RUN_TEST(analysis_records_raw_data_setup_time);
        RUN_TEST(analysis_adjusts_data_setup_time);
        RUN_TEST(analysis_records_raw_data_hold_time);
        RUN_TEST(analysis_adjusts_data_hold_time);
        RUN_TEST(analysis_records_raw_data_valid_time);
        RUN_TEST(analysis_adjusts_data_valid_time);
    }

    I2CTimingAnalyserTest() : TestSuite(__FILE__) {};
};
common::hal::FakeClock I2CTimingAnalyserTest::clock;
} // analysis
#endif //I2C_UNDERNEATH_I2C_TIMING_ANALYSER_TEST_H
