# abls-agent-ups

Standalone UPS runtime for Abls-Habitat.

## Current implementation status

- Runtime based on `abls-agent-libs`
- Keeps SRC `Watchdogd/Onduleur` business logic:
  - connection to a NUT server (`upsd`) through `libupsclient`, optional `USERNAME`/`PASSWORD` login
  - 10 AI published through MQTT (`LOAD`, `REALPOWER`, `BATTERY_CHARGE`, `INPUT_VOLTAGE`,
    `BATTERY_RUNTIME`, `BATTERY_VOLTAGE`, `INPUT_HZ`, `OUTPUT_CURRENT`, `OUTPUT_HZ`, `OUTPUT_VOLTAGE`)
  - 7 DI derived from `ups.status` and the outlet states (`UPS_ONLINE`, `UPS_CHARGING`, `UPS_ON_BATT`,
    `UPS_REPLACE_BATT`, `UPS_ALARM`, `OUTLET_1_STATUS`, `OUTLET_2_STATUS`)
  - 9 monostable DO mapped to NUT instant commands (`LOAD_ON`/`LOAD_OFF`, outlets, battery tests)
  - adaptive polling: every 10s when connected, retry every 180s otherwise
  - communication status reporting to master (`IO_COMM`)

Expected API config fields:

- `host` (address of the NUT server)
- `name` (UPS name declared in `ups.conf`)
- `admin_username` / `admin_password` (optional NUT credentials)

They can also be provided locally, through `/etc/abls-agent.conf`, environment
(`ABLS_HOST`, `ABLS_NAME`, `ABLS_ADMIN_USERNAME`, `ABLS_ADMIN_PASSWORD`) or command line
(`--host=`, `--name=`, `--admin-username=`, `--admin-password=`).

## Build

```sh
./install_deps.sh
./build.sh
```

## Packaging RPM

```sh
./build_rpm.sh
```

Produces runtime RPM package in `build/`.

The runtime package also installs a templated systemd unit:

- `abls-agent-ups@.service`

Start one instance per agent tech id:

```sh
sudo systemctl enable --now abls-agent-ups@<agent-tech-id>.service
```

## Packaging DEB

```sh
./build_apt.sh --dist bookworm
./build_apt.sh --dist trixie
```

Default target suite is detected from host OS codename (`/etc/os-release`), with `bookworm` fallback.

Produces runtime DEB package and copies normalized artifacts to:

- `build/pkgs/deb/<suite>/<arch>/`

`build_apt.sh` builds only the native host architecture.

Package signatures are centralized in ABLS-PKGS (both DEB repository metadata and RPM package/repository signatures).

The DEB package installs the same templated systemd unit:

```sh
sudo systemctl enable --now abls-agent-ups@<agent-tech-id>.service
```

## Release bump + publication

```sh
./bump.sh 1.2.3
```

The release flow:

- tags `v1.2.3` from `trunk`
- merges `trunk` into `main`
- builds RPM + DEB packages
- copies RPM to `../ABLS-PKGS/public/rpms/<arch>/`

## Container build

```sh
podman build -t abls-agent-ups:dev \
  --build-arg ABLS_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_LIBS_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_RPM_URL=<url> \
  -f Containerfile .
```
