#!/usr/bin/env bash
set -euo pipefail

uid="$(id -u docker)"
install -d -m 0755 /run/xrdp /run/xrdp/sockdir
install -d -m 0700 -o docker -g docker "/run/user/${uid}"
rm -f /run/xrdp/*.pid /var/run/xrdp/*.pid

mkdir -p /run/dbus
rm -f /run/dbus/pid
dbus-daemon --system --fork

if [[ -x /usr/sbin/sshd ]]; then
  /usr/sbin/sshd
fi

xrdp-sesman --nodaemon --config /etc/xrdp/sesman.ini &
exec xrdp --nodaemon --config /etc/xrdp/xrdp.ini
