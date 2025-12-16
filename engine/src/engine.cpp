#include "dr_api.h"
#include "drmgr.h"
#include "drutil.h"
#include "drwrap.h"
#include "drsyms.h"

#include <unordered_map>
#include <cstdint>

static std::unordered_map<app_pc, uint64_t> func_call_count;
static std::unordered_map<app_pc, uint64_t> mem_access_count;

static bool hotspot_detected = false;
static bool patch_applied   = false;

static app_pc slow_func_addr = nullptr;
static app_pc fast_func_addr = nullptr;

/* ---------- clean calls ---------- */

static void count_function(void *, app_pc pc) {
    func_call_count[pc]++;
}

static void count_memory_access(void *, app_pc pc) {
    mem_access_count[pc]++;
}

/* ---------- BB instrumentation ---------- */

static dr_emit_flags_t
event_bb_instrumentation(void *, void *, instrlist_t *bb,
                         instr_t *instr, bool, bool, void *)
{
    if (instr != instrlist_first_app(bb))
        return DR_EMIT_DEFAULT;

    app_pc pc = instr_get_app_pc(instr);

    dr_insert_clean_call(nullptr, bb, instr,
        (void *)count_function, false, 1, OPND_CREATE_INTPTR(pc));

    if (instr_reads_memory(instr) || instr_writes_memory(instr)) {
        dr_insert_clean_call(nullptr, bb, instr,
            (void *)count_memory_access, false, 1, OPND_CREATE_INTPTR(pc));
    }

    return DR_EMIT_DEFAULT;
}

/* ---------- patch ---------- */

static void apply_patch() {
    if (patch_applied || !slow_func_addr || !fast_func_addr)
        return;

    drwrap_replace(slow_func_addr, fast_func_addr, false);
    patch_applied = true;

    dr_printf("[PATCH] slow_function redirected to fast_function\n");
}

/* ---------- hotspot detection ---------- */

static void detect_hotspot() {
    uint64_t total_calls = 0, total_mem = 0;

    for (auto &e : func_call_count) total_calls += e.second;
    for (auto &e : mem_access_count) total_mem += e.second;

    if (!total_calls || !total_mem) return;

    for (auto &e : func_call_count) {
        double call_ratio = (double)e.second / total_calls;
        double mem_ratio  = (double)mem_access_count[e.first] / total_mem;

        if (call_ratio > 0.30 || mem_ratio > 0.40) {
            hotspot_detected = true;
            dr_printf("[HOTSPOT] %p call=%.2f mem=%.2f\n",
                      e.first, call_ratio, mem_ratio);
            apply_patch();
            return;
        }
    }
}

/* ---------- module load ---------- */

static void
event_module_load(void *, const module_data_t *mod, bool)
{
    size_t offset;

    if (!slow_func_addr &&
        drsym_lookup_symbol(mod->full_path,
            "slow_function", &offset, 0) == DRSYM_SUCCESS)
        slow_func_addr = mod->start + offset;

    if (!fast_func_addr &&
        drsym_lookup_symbol(mod->full_path,
            "fast_function", &offset, 0) == DRSYM_SUCCESS)
        fast_func_addr = mod->start + offset;
}

/* ---------- exit ---------- */

static void event_exit(void) {
    detect_hotspot();
    drsym_exit();
}

/* ---------- entry ---------- */

DR_EXPORT void
dr_client_main(client_id_t, int, const char **) {
    drmgr_init();
    drutil_init();
    drwrap_init();
    drsym_init(0);

    drmgr_register_bb_instrumentation_event(
        nullptr, event_bb_instrumentation, nullptr);
    drmgr_register_module_load_event(event_module_load);
    dr_register_exit_event(event_exit);

    dr_printf("WiseHack25 Engine loaded (FINAL)\n");
}
