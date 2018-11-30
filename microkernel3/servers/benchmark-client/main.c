#include <resea.h>
#include <resea/benchmark.h>
#include <resea/discovery.h>
#include <resea/logging.h>

static inline u64_t rdtscp(u32_t *cpu) {
    u64_t rax, rdx;

    __asm__ __volatile__(
        "cpuid      \n"
        "rdtscp     \n"
    : "=a"(rax), "=d"(rdx), "=c"(*cpu)
    :
    : "%rbx"
    );

    return (rdx << 32) | rax;
}

static inline u64_t rdpmc(u32_t reg) {
    u32_t low, high;
    __asm__ __volatile__("rdpmc" : "=a"(low), "=d"(high) : "c"(reg));
    return ((u64_t) high << 32) | low;
}

#define TEST_NUM 128
struct result {
    u64_t rdtsc;
    u64_t cycles;
    u64_t llc_miss;
    u64_t stlb_miss;
};
struct result results[TEST_NUM];

static inline void bench_start(int i) {
    u32_t cpu;
    results[i].rdtsc = rdtscp(&cpu);
    results[i].cycles = rdpmc(0);
    results[i].llc_miss = rdpmc(1);
    results[i].stlb_miss = rdpmc(2);
}

static inline void bench_end(int i) {
    u32_t cpu;
    results[i].rdtsc = rdtscp(&cpu) - results[i].rdtsc;
    results[i].cycles = rdpmc(0) - results[i].cycles;
    results[i].llc_miss = rdpmc(1) - results[i].llc_miss;
    results[i].stlb_miss = rdpmc(2) - results[i].stlb_miss;
}

#define SUMMARIZE(attr) \
    do {                                                            \
        u64_t min = 0xffffffffffffffff, max = 0, mean = 0;          \
        for (int i = 0; i < TEST_NUM; i++) {                        \
            u64_t r = results[i].attr;                              \
                                                                    \
            if (r < min)                                            \
                min = r;                                            \
            if (r > max)                                            \
                max = r;                                            \
                                                                    \
            mean += r;                                              \
        }                                                           \
                                                                    \
        /* Sort the results in order to obtain these parameters.*/  \
        u64_t median, q1, q3;                                       \
        bool swapped = true;                                        \
        while (swapped) {                                           \
            swapped = false;                                        \
            for (int i = 0; i < TEST_NUM - 1; i++) {                \
                if (results[i].attr > results[i + 1].attr) {        \
                    u64_t tmp = results[i + 1].attr;                \
                    results[i + 1].attr = results[i].attr;          \
                    results[i].attr = tmp;                          \
                    swapped = true;                                 \
                }                                                   \
            }                                                       \
        }                                                           \
                                                                    \
        q1 = results[TEST_NUM / 4].attr;                            \
        q3 = results[(TEST_NUM / 4) * 3].attr;                      \
        median = results[TEST_NUM / 2].attr;                        \
        mean /= TEST_NUM;                                           \
        printf("{name: \"%s\", type: \"%s\", min: %d, q1: %d, q3: %d, max: %d}\n", \
            name, #attr, min, q1, q3, max); \
    } while(0)

void bench_summarize(const char *name) {
//    SUMMARIZE(rdtsc);
    SUMMARIZE(cycles);
    SUMMARIZE(llc_miss);
//    SUMMARIZE(stlb_miss);
}

void rdtscp_latency_benchmark(void) {
    for (int i = 0; i < TEST_NUM; i++) {
        bench_start(i);
        bench_end(i);
    }

    bench_summarize("rdtscp");
}

void rdpmc_latency_benchmark(void) {
    for (int i = 0; i < TEST_NUM; i++) {
        u64_t start = rdpmc(0);
        u64_t end = rdpmc(0);

        results[i].rdtsc = 0;
        results[i].cycles = end - start;
        results[i].llc_miss = 0;
        results[i].stlb_miss = 0;
    }

    bench_summarize("rdpmc0");
}

void rpc_latency_benchmark(void) {
    channel_t server;
    call_discovery_discover(1, BENCHMARK_SERVICE, &server);

    payload_t a0 = 0xabcdef000000004;
    payload_t a1 = 0xabcdef000000001;
    payload_t a2 = 0xabcdef000000002;
    payload_t a3 = 0xabcdef000000003;

    for (int i = 0; i < TEST_NUM; i++) {
        payload_t r;
        bench_start(i);
        ipc_call(server, 0, a0, a1, a2, a3, &r, &r, &r, &r);
        bench_end(i);
    }

    bench_summarize("ipc-roundtrip");

    a0 = (payload_t) "Hello";
    a1 = 5;

    for (int i = 0; i < TEST_NUM; i++) {
        payload_t r;
        bench_start(i);
        ipc_call(server, 1, a0, a1, a2, a3, &r, &r, &r, &r);
        bench_end(i);
    }

    bench_summarize("small-ool-ipc-roundtrip");
}

void syscall_latency_benchmark(void) {
    for (int i = 0; i < TEST_NUM; i++) {
        payload_t r;
        bench_start(i);
        ipc_close(0);
        bench_end(i);
    }

    bench_summarize("syscall");
}

void main(void) {
    printf("\nBenchmarks are being started. It's time to brew coffee!\n\n");
    rdtscp_latency_benchmark();
    rdpmc_latency_benchmark();
    rpc_latency_benchmark();
    syscall_latency_benchmark();
    printf("\nFinished all benchmarks.\n");
}
