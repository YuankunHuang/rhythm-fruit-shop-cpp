#include "rhythm/ChartLoader.h"
#include "rhythm/ChartCatalog.h"
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
        auto chart = loader.Load("assets/charts/test_fixture.rfs.json", "easy", err);
        EXPECT(chart.has_value());
        if (chart) {
            EXPECT(chart->Title() == "Test Fixture");
            EXPECT(chart->Notes().size() == 4);
            EXPECT(chart->LaneCount() == 4);
            EXPECT(chart->ApproachTimeMs() == 1600);
        }
    }

    // --- ChartLoader: valid chart, expert difficulty ---
    {
        rfs::ChartLoader loader;
        rfs::LoadError err{};
        auto chart = loader.Load("assets/charts/test_fixture.rfs.json", "expert", err);
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
        auto chart = loader.Load("assets/charts/test_fixture.rfs.json", "hard", err);
        EXPECT(!chart.has_value());
        EXPECT(err.code == "diff_not_found");
    }

    // --- ChartCatalog: empty catalog ---
    {
        rfs::ChartCatalog catalog;
        std::string err{};
        catalog = rfs::ChartCatalog::Load("assets/charts/catalog.json", err);
        EXPECT(catalog.IsValid());
        EXPECT(catalog.Songs().empty()); // catalog.json starts empty
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }
    std::cout << "All tests passed.\n";
    return 0;
}
