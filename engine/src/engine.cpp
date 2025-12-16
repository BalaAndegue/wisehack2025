#include "dr_api.h"
#include "drmgr.h"
#include "drutil.h"

#include <unordered_map>
#include <cstdint>

/* =======================
   Structures globales
   ======================= */

static std::unordered_map<app_pc, uint64_t> func_call_count;
static std::unordered_map<app_pc, uint64_t> mem_access_count;

/* =======================
   Clean calls
   ======================= */

static void
count_function(void *drcontext, app_pc func_addr)
{
    func_call_count[func_addr]++;
}

static void
count_memory_access(void *drcontext, app_pc func_addr)
{
    mem_access_count[func_addr]++;
}

/* =======================
   Instrumentation BB
   ======================= */

static dr_emit_flags_t
event_bb_instrumentation(void *drcontext,
                          void *tag,
                          instrlist_t *bb,
                          instr_t *instr,
                          bool for_trace,
                          bool translating,
                          void *user_data)
{
    /* On ne traite que la première instruction du BB */
    if (instr != instrlist_first_app(bb))
        return DR_EMIT_DEFAULT;

    /* Adresse de la fonction courante */
    app_pc pc = instr_get_app_pc(instr);

    /* ---- Comptage appels ---- */
    dr_insert_clean_call(
        drcontext,
        bb,
        instr,
        (void *)count_function,
        false,
        1,
        OPND_CREATE_INTPTR(pc)
    );

    /* ---- Comptage accès mémoire ---- */
    if (instr_reads_memory(instr) || instr_writes_memory(instr)) {
        dr_insert_clean_call(
            drcontext,
            bb,
            instr,
            (void *)count_memory_access,
            false,
            1,
            OPND_CREATE_INTPTR(pc)
        );
    }

    return DR_EMIT_DEFAULT;
}

/* =======================
   Sortie programme
   ======================= */

static void
event_exit(void)
{
    dr_printf("\n=== Function Call Summary ===\n");
    for (auto &it : func_call_count) {
        dr_printf("Function %p called %llu times\n",
                  it.first,
                  (unsigned long long)it.second);
    }

    dr_printf("\n=== Memory Access Summary ===\n");
    for (auto &it : mem_access_count) {
        dr_printf("Function %p accessed memory %llu times\n",
                  it.first,
                  (unsigned long long)it.second);
    }
}

/* =======================
   Entrée client
   ======================= */

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[])
{
    drmgr_init();
    drutil_init();

    dr_register_exit_event(event_exit);

    drmgr_register_bb_instrumentation_event(
        NULL,
        event_bb_instrumentation,
        NULL
    );

    dr_printf("WiseHack25 Engine loaded (Phase 1 + Memory tracking)\n");
}
