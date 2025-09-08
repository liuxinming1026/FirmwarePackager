#include <archive.h>
#include <archive_entry.h>
#include <nlohmann/json.hpp>

int core_placeholder() {
    struct archive* a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_free(a);

    nlohmann::json j;
    return j.is_null();
}
