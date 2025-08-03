# generate_secrets.py
env = {}
with open(".env") as f:
    for line in f:
        if "=" in line:
            k, v = line.strip().split("=", 1)
            env[k] = v

with open("src/secrets.h", "w") as f:
    for k, v in env.items():
        f.write(f'#define {k} "{v}"\n')
