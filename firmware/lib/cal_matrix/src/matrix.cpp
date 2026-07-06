#include <cal/matrix.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

namespace cal {

namespace {

constexpr char kPath[] = "/cal_matrix.json";

// Default matrix — bench lab, Dorinda cube @ ~12 V not charging (2026-07-05).
// Traceability: docs/test-log.md, firmware/data/cal_matrix.json (meta.source=bench_lab).
// PWM from V/12 × 1023; 426 ≈ 5 V sweet spot (~1.7 A).
constexpr Row kDefaultRows[] = {
    {86, 327},
    {205, 632},
    {410, 1507},
    {426, 1700},
    {512, 2080},
    {614, 2600},
    {819, 4000},
    {1023, 5560},
};
constexpr size_t kDefaultRowCount = sizeof(kDefaultRows) / sizeof(kDefaultRows[0]);

Row g_rows[MAX_ROWS];
size_t g_count = 0;

void sort_rows()
{
    for (size_t i = 0; i + 1 < g_count; ++i)
    {
        for (size_t j = i + 1; j < g_count; ++j)
        {
            if (g_rows[j].pwm < g_rows[i].pwm)
            {
                const Row tmp = g_rows[i];
                g_rows[i] = g_rows[j];
                g_rows[j] = tmp;
            }
        }
    }
}

bool seed_defaults()
{
    g_count = 0;
    for (size_t i = 0; i < kDefaultRowCount && g_count < MAX_ROWS; ++i)
    {
        g_rows[g_count] = kDefaultRows[i];
        ++g_count;
    }
    sort_rows();
    return save();
}

}  // namespace

bool init()
{
    if (!LittleFS.begin())
    {
        Serial.println("LittleFS mount failed");
        return false;
    }
    if (!load())
    {
        return false;
    }
    if (g_count == 0)
    {
        Serial.println("cal_matrix: seeding Dorinda bench defaults");
        return seed_defaults();
    }
    return true;
}

bool load_embedded_defaults()
{
    g_count = 0;
    for (size_t i = 0; i < kDefaultRowCount && g_count < MAX_ROWS; ++i)
    {
        g_rows[g_count] = kDefaultRows[i];
        ++g_count;
    }
    sort_rows();
    return g_count > 0;
}

bool load()
{
    g_count = 0;
    if (!LittleFS.exists(kPath))
    {
        return true;
    }

    File f = LittleFS.open(kPath, "r");
    if (!f)
    {
        return false;
    }

    JsonDocument doc;
    const DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err)
    {
        Serial.printf("cal_matrix load parse error: %s\n", err.c_str());
        return false;
    }

    JsonArray arr = doc["rows"].as<JsonArray>();
    for (JsonObject row : arr)
    {
        if (g_count >= MAX_ROWS)
        {
            break;
        }
        g_rows[g_count].pwm = row["pwm"] | 0;
        g_rows[g_count].current_ma = row["current_ma"] | 0;
        ++g_count;
    }
    sort_rows();
    return true;
}

bool save()
{
    JsonDocument doc;
    JsonArray arr = doc["rows"].to<JsonArray>();
    for (size_t i = 0; i < g_count; ++i)
    {
        JsonObject row = arr.add<JsonObject>();
        row["pwm"] = g_rows[i].pwm;
        row["current_ma"] = g_rows[i].current_ma;
    }

    File f = LittleFS.open(kPath, "w");
    if (!f)
    {
        return false;
    }
    if (serializeJson(doc, f) == 0)
    {
        f.close();
        return false;
    }
    f.close();
    return true;
}

bool upsert(uint16_t pwm, uint16_t current_ma)
{
    for (size_t i = 0; i < g_count; ++i)
    {
        if (g_rows[i].pwm == pwm)
        {
            g_rows[i].current_ma = current_ma;
            return save();
        }
    }
    if (g_count >= MAX_ROWS)
    {
        return false;
    }
    g_rows[g_count].pwm = pwm;
    g_rows[g_count].current_ma = current_ma;
    ++g_count;
    sort_rows();
    return save();
}

size_t count()
{
    return g_count;
}

const Row* rows()
{
    return g_rows;
}

int interpolate_ma(uint16_t pwm)
{
    if (g_count == 0)
    {
        return -1;
    }
    if (pwm <= g_rows[0].pwm)
    {
        return static_cast<int>(g_rows[0].current_ma);
    }
    if (pwm >= g_rows[g_count - 1].pwm)
    {
        return static_cast<int>(g_rows[g_count - 1].current_ma);
    }
    for (size_t i = 0; i + 1 < g_count; ++i)
    {
        const Row& a = g_rows[i];
        const Row& b = g_rows[i + 1];
        if (pwm >= a.pwm && pwm <= b.pwm)
        {
            const int span = static_cast<int>(b.pwm) - static_cast<int>(a.pwm);
            if (span <= 0)
            {
                return static_cast<int>(a.current_ma);
            }
            const int t = static_cast<int>(pwm) - static_cast<int>(a.pwm);
            const int ma_span = static_cast<int>(b.current_ma) - static_cast<int>(a.current_ma);
            return static_cast<int>(a.current_ma) + (ma_span * t) / span;
        }
    }
    return -1;
}

void dump_serial()
{
    Serial.printf("Matrix updated (%u rows):\n", static_cast<unsigned>(g_count));
    for (size_t i = 0; i < g_count; ++i)
    {
        Serial.printf("  PWM=%5u  -> %4u mA\n", g_rows[i].pwm, g_rows[i].current_ma);
    }
}

void clear()
{
    g_count = 0;
    LittleFS.remove(kPath);
}

}  // namespace cal
