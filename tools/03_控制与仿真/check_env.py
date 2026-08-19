import sys
print("python:", sys.version.split()[0])
for mod in ("numpy", "torch", "cupy", "jax", "numba", "scipy"):
    try:
        m = __import__(mod)
        ver = getattr(m, "__version__", "?")
        extra = ""
        if mod == "torch":
            extra = f" cuda_available={m.cuda.is_available()} devices={m.cuda.device_count()}"
        elif mod == "cupy":
            extra = f" cuda_runtime={m.cuda.runtime.runtimeGetVersion()}"
        print(f"{mod}: {ver}{extra}")
    except Exception as e:
        print(f"{mod}: MISSING ({type(e).__name__})")
