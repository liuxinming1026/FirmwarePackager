# Installer State Machine

The generated `install.sh` script performs upgrades using a small
state machine.  Progress is persisted under `/opt/upgrade/state` to
survive power loss or reboots.

## States

* **init** – First entry point.  The package archive is copied to
  `/opt/upgrade/packages/<PKG_ID>.tar.gz` and the next state is saved.
* **copy** – Individual payload files are installed.  Each successfully
  installed file is appended to the journal file
  `/opt/upgrade/state/<PKG_ID>.journal`.
* **rollback** – Invoked when an error occurs.  The journal is replayed
  to remove previously installed files.
* **done** – Installation completed.  State and journal files are
  removed.

The current state is stored in `/opt/upgrade/state/<PKG_ID>.state`.
`recover_boot.sh` scans this directory on boot and replays unfinished
installations by running the corresponding `install.sh --resume`.
