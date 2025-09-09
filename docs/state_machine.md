# Installer State Machine

The generated `install.sh` script performs upgrades using a small
state machine.  Progress is persisted under `/opt/upgrade/state` and
`/opt/upgrade/journal` to survive power loss or reboots.

## States

* **PREPARE** – First entry point. The package archive is copied to
  `/opt/upgrade/packages/<PKG_ID>.tar.gz` and recovery hooks are
  installed.
* **BACKUP** – Individual payload files are verified, backed up and
  replaced. Each successfully installed file is appended to the journal
  file `/opt/upgrade/journal/<PKG_ID>.journal`.
* **COMMIT** – All files have been applied and the system is ready to
  finalize the upgrade.
* **DONE** – Installation completed. State and journal files are
  removed.
* **FAILED** – An error occurred and rollback was executed. The state
  file is kept for inspection.

In addition to the step above, the state file contains a `STATUS`
field describing the overall progress. Valid values are:

* `IN_PROGRESS` – Normal upgrade flow.
* `ROLLBACK` – A rollback is in progress before the upgrade is
  attempted again.
* `SUCCESS` – Upgrade completed successfully.
* `FAIL` – Upgrade failed even after rollback.

When `recover_boot.sh` encounters a state file with `STATUS=ROLLBACK`,
it invokes the package's `install.sh` script twice: first to complete
the rollback and then again to restart the upgrade process.

The current step is stored in `/opt/upgrade/state/<PKG_ID>.state` which
contains the fields `STEP`, `JOURNAL`, `PKG_TGZ` and `LAST_FILE`.
`recover_boot.sh` scans this directory on boot and replays unfinished
installations by running the corresponding `install.sh --resume`.
