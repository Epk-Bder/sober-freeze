# sober-freeze

A tiny utility/example for freezing and unfreezing [sober](https://sober.vinegarhq.org/) using [cgroup v2](https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html) without triggering the anti-tamper.

It targets the process named `Main` (as used by Sober) and suspends it by moving it into a temporary frozen cgroup.

---

you need this:

- Sober
- Linux with **cgroup v2 enabled**
- GCC :cold_sweat:
- A computer


## build

```bash
make
```

## why would you want this?

for cool glitches/mechanics like lag high jumping and uh laugh clipping and stuff and cool obbyist stuff like very epic stuff, like you know. epic stuff. like yeah. you know what i mean. (if you're using this to do skips in towers you're a fraud)