#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "sim.h"

namespace {
constexpr unsigned S   = 1000;
constexpr unsigned MIN = 60 * S;
constexpr unsigned QUICK = 750;
constexpr unsigned SLOW = 1250;

// Runs each test twice: from t=0 and from just before millis() wraps.
struct Fixture {
    Fixture() {
        const uint32_t start = GENERATE(0u, 0xFFFFFF00u);
        sim_reset(start);
    }
};
}

TEST_CASE_METHOD(Fixture, "fan is off after reset") {
    sim_advance(10 * S);
    CHECK_FALSE(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "fan turns on after the light has been on for 3 minutes") {
    sim_set_light(true);
    sim_advance(3 * MIN - 500);
    CHECK_FALSE(sim_fan_pin());
    sim_advance(1 * S);
    CHECK(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "short light use never starts the fan") {
    sim_set_light(true);  sim_advance(2 * MIN);
    sim_set_light(false); sim_advance(10 * MIN);
    CHECK_FALSE(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "fan keeps running 2 minutes after the light goes off") {
    sim_set_light(true);  sim_advance(3 * MIN + S);
    sim_set_light(false); sim_advance(2 * MIN - S);
    CHECK(sim_fan_pin());
    sim_advance(2 * S);
    CHECK_FALSE(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "turning the light back on cancels the off delay") {
    sim_set_light(true);  sim_advance(3 * MIN + S);
    sim_set_light(false); sim_advance(1 * MIN);
    sim_set_light(true);  sim_advance(5 * MIN);
    CHECK(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "fan is off, turning the light on and off quickly turns on the fan") {
    sim_advance(10 * S);
    sim_set_light(true);  sim_advance(QUICK);
    sim_set_light(false); sim_advance(QUICK);
    CHECK(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "fan is off, toggling light quickly turns on fan, next toggles ignored") {
    sim_advance(10 * S);
    sim_set_light(true);  sim_advance(QUICK);
    sim_set_light(false); sim_advance(QUICK);
    sim_set_light(true);  sim_advance(QUICK);
    CHECK(sim_fan_pin());
    sim_set_light(false); sim_advance(QUICK);
    CHECK(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "fan is off, turning the light on and off slowly keeps fan off") {
    sim_advance(10 * S);
    sim_set_light(true);  sim_advance(SLOW);
    sim_set_light(false); sim_advance(SLOW);
    CHECK_FALSE(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "fan is on, turning the light off and on quickly turns off the fan permanently") {
    sim_set_light(true);
    sim_advance(4 * MIN);
    sim_set_light(false); sim_advance(QUICK);
    sim_set_light(true);  sim_advance(QUICK);
    CHECK_FALSE(sim_fan_pin());
    sim_advance(4 * MIN);
    CHECK_FALSE(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "fan is on, turning the light off and on slowly keeps fan on") {
    sim_set_light(true);
    sim_advance(4 * MIN);
    CHECK(sim_fan_pin());
    sim_set_light(false); sim_advance(SLOW);
    sim_set_light(true);  sim_advance(SLOW);
    CHECK(sim_fan_pin());
}

TEST_CASE_METHOD(Fixture, "contact bounce is ignored") {
    for (int i = 0; i < 10; i++) {
        sim_set_light(true);  sim_advance(3);
        sim_set_light(false); sim_advance(3);
    }
    sim_advance(5 * MIN);
    CHECK_FALSE(sim_fan_pin());
}
