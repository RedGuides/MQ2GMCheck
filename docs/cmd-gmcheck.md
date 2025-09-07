---
tags:
  - command
---

# /gmcheck

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/gmcheck [on|off] [option [on|off]]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Lists past GM sightings, and toggles various settings to alert you about fun and exciting GM events in your zone.
<!--cmd-desc-end-->

## Options

| Option | Description |
|--------|-------------|
| `on|off` | enable/disable scanning for GMs |
| `chat [on|off]` | toggle chat channel alert |
| `chattimer <#>` | set frequency of chat channel alert, default=15s |
| `popup [on|off]` | toggle popup alert |
| `popuptimer <#>` | set frequency of popup alert, default=30s |
| `audio [on|off]` | toggle audio alert |
| `audiotimer <#>` | set frequency of audio alert, default=30s |
| `testaudio` | command to test if your audio is working correctly |
| `list` | list all known GMs currently in the zone |
| `reset` | reset list of known GMs |
| `history` | complete history dump |
| `zone` | History of GMs in the zone |
| `server` | History of GMs on the server |
| `servers` | History of GMs on all servers |
| `timer <#>` | set scan delay to X seconds, default=2s |
| `help` | show available commands |
