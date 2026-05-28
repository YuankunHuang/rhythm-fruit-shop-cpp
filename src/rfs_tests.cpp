#include "rhythm/ChartLoader.h"
#include "rhythm/ChartCatalog.h"
#include "rhythm/AudioPathResolver.h"
#include "rhythm/SongDisplay.h"
#include "rhythm/SmoothedSongClock.h"
#include "platform/SampleAnchor.h"
#include <cstdlib>
#include <iostream>

#define EXPECT(cond) do { \
    if (!(cond)) { \
        std::cerr << "FAIL: " #cond << " @ " << __FILE__ << ":" << __LINE__ << '\n'; \
        ++failures; \
    } \
} while (0)

int main() {
    int failures = 0;

    // --- ChartLoader: valid chart, easy difficulty ---
    {
        rfs::ChartLoader loader;
        rfs::LoadError err{};
        auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "easy", err);
        EXPECT(chart.has_value());
        if (chart) {
            EXPECT(chart->Notes().size() == 4);
            EXPECT(chart->LaneCount() == 4);
            EXPECT(chart->ApproachTimeMs() == 1600);
            const auto& notes = chart->Notes();
            for (std::size_t i = 1; i < notes.size(); ++i) {
                EXPECT(notes[i - 1].time_ms <= notes[i].time_ms);
            }
        }
    }

    // --- ChartLoader: valid chart, expert difficulty ---
    {
        rfs::ChartLoader loader;
        rfs::LoadError err{};
        auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "expert", err);
        EXPECT(chart.has_value());
        if (chart) {
            EXPECT(chart->Notes().size() == 6);
        }
    }

    // --- ChartLoader: missing file ---
    {
        rfs::ChartLoader loader;
        rfs::LoadError err{};
        auto chart = loader.Load("assets/charts/does_not_exist.rfs.json", "easy", err);
        EXPECT(!chart.has_value());
        EXPECT(err.code == "file_not_found");
    }

    // --- ChartLoader: wrong difficulty key ---
    {
        rfs::ChartLoader loader;
        rfs::LoadError err{};
        auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "hard", err);
        EXPECT(!chart.has_value());
        EXPECT(err.code == "diff_not_found");
    }

    // --- ChartCatalog: rebuilt catalog ---
    {
        std::string err{};
        auto catalog = rfs::ChartCatalog::Load("assets/charts/catalog.json", err);
        EXPECT(catalog.IsValid());
        EXPECT(!catalog.Songs().empty());
    }

    // --- AudioPathResolver ---
    {
        const auto lemon = rfs::AudioPathResolver::Resolve("lemon-water-light");
        EXPECT(lemon.has_value());
        if (lemon) {
            EXPECT(lemon->generic_string().find("lemon-water-light") != std::string::npos);
        }

        const auto lets_drive = rfs::AudioPathResolver::Resolve("lets-drive");
        EXPECT(lets_drive.has_value());
        if (lets_drive) {
            EXPECT(lets_drive->generic_string().find("lets-drive") != std::string::npos);
        }

        const auto drama = rfs::AudioPathResolver::Resolve("drama");
        EXPECT(drama.has_value());
    }

    // --- SongDisplay ---
    {
        EXPECT(rfs::HumanizeSongId("lemon-water-light") == "Lemon Water Light");
        rfs::SongEntry entry{};
        entry.id = "lets-drive";
        entry.title = "source";
        EXPECT(rfs::DisplaySongTitle(entry) == "Lets Drive");
    }

    // --- SmoothedSongClock: freeze ---
    {
        rfs::SmoothedSongClock clock{};
        rfs::SampleAnchor anchor{};
        anchor.sample_rate = 48000;
        anchor.sample_cursor = 48000;
        anchor.host_ns = 1'000'000'000;
        clock.Tick(anchor, anchor.host_ns);
        
        const rfs::HostNanos host_ns_at_pause = anchor.host_ns + 500'000'000;
        const float t0 = clock.NowMs(host_ns_at_pause);
        
        clock.SetFrozen(t0, host_ns_at_pause);

        const rfs::HostNanos host_ns_after_pause = host_ns_at_pause + 3'000'000'000;
        EXPECT(t0 == clock.NowMs(host_ns_after_pause)); // when frozen, clock.NowMs stays unchanged

        clock.ClearFrozen(host_ns_after_pause);

        EXPECT(clock.NowMs(host_ns_after_pause) == t0); // moment of defreezing, no gap
        EXPECT(clock.NowMs(host_ns_after_pause + 16'000'000) == t0 + 16.f);
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }
    std::cout << "All tests passed.\n";
    return 0;
}

