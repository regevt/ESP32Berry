# MicroPython-specific helpers
import os

def listdir(path='/'):
    try:
        return os.listdir(path)
    except Exception:
        try:
            # Fallback for ports supporting ilistdir with tuples
            return [e[0] for e in os.ilistdir(path)]  # type: ignore[attr-defined]
        except Exception:
            return []
