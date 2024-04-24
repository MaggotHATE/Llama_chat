#pragma once
#include <stddef.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

struct ggml_tensor;
struct ggml_compute_params;

bool llamafile_sgemm(int, int, int, const void *, int, const void *, int,
                     void *, int, int, int, int, int, int, int);

bool llamafile_mixmul(const struct ggml_compute_params *, const struct ggml_tensor *,
                      const struct ggml_tensor *, const struct ggml_tensor *,
                      struct ggml_tensor *);

size_t llamafile_mixmul_needs(const struct ggml_tensor *,
                              const struct ggml_tensor *,
                              const struct ggml_tensor *);

#ifdef __cplusplus
}
#endif
