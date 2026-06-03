#include <doctest/doctest.h>
#include "rhythm/ChartLoader.h"
#include "rhythm/ChartCatalog.h"
#include "rhythm/AudioPathResolver.h"
#include "rhythm/SongDisplay.h"

// --- ChartLoader: valid chart, easy difficulty ---
TEST_CASE("ChartLoader - loads test-fixture easy") {
    rfs::ChartLoader loader;
    rfs::LoadError err{};
    auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "easy", err);
    REQUIRE(chart.has_value());
    CHECK(chart->Notes().size() == 4);
    CHECK(chart->LaneCount() == 4);
    CHECK(chart->ApproachTimeMs() == 1600);
    const auto& notes = chart->Notes();
    for (std::size_t i = 1; i < notes.size(); ++i) {
        CHECK(notes[i - 1].time_ms <= notes[i].time_ms);
    }
}

// --- ChartLoader: missing file ---
TEST_CASE("ChartLoader - rejects missing file") {
    rfs::ChartLoader loader;
    rfs::LoadError err{};
    auto chart = loader.Load("assets/charts/does_not_exist.rfs.json", "easy", err);
    CHECK_FALSE(chart.has_value());
    CHECK(err.code == "file_not_found");
}

// --- ChartLoader: wrong difficulty key ---
TEST_CASE("ChartLoader - rejects missing difficulty") {
    rfs::ChartLoader loader;
    rfs::LoadError err{};
    auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "hard", err);
    CHECK_FALSE(chart.has_value());
    CHECK(err.code == "diff_not_found");
}

// --- ChartCatalog: rebuilt catalog ---
TEST_CASE("ChartLoader - validates catalog building") {
    std::string err{};
    auto catalog = rfs::ChartCatalog::Load("assets/charts/catalog.json", err);
    CHECK(catalog.IsValid());
    CHECK(!catalog.Songs().empty());
}

TEST_CASE("AudioPathResolver - resolves sample clips (service/track)") {
    const auto lemon = rfs::AudioPathResolver::Resolve("lemon-water-light");
    REQUIRE(lemon.has_value());
    CHECK(lemon->generic_string().find("lemon-water-light") != std::string::npos);

    const auto lets_drive = rfs::AudioPathResolver::Resolve("lets-drive");
    REQUIRE(lets_drive.has_value());
    CHECK(lets_drive->generic_string().find("lets-drive") != std::string::npos);

    const auto drama = rfs::AudioPathResolver::Resolve("drama");
    REQUIRE(drama.has_value());
}

TEST_CASE("SongDisplay - properly parses and displays song info") {
    CHECK(rfs::HumanizeSongId("lemon-water-light") == "Lemon Water Light");
    rfs::SongEntry entry{};
    entry.id = "lets-drive";
    entry.title = "source";
    CHECK(rfs::DisplaySongTitle(entry) == "Lets Drive");
}