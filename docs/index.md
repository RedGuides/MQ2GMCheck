---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2gmcheck.131/"
support_link: "https://www.redguides.com/community/threads/mq2gmcheck.66836/"
repository: "https://github.com/MMOBugs/MQ2GMCheck"
config: "MQ2GMCheck.ini"
authors: "Bushdaka"
tagline: "Helps identify GMs, but only if they want to be detected."
---

# MQ2GMCheck

<!--desc-start-->
This plugin checks to see if a visible GM is in the zone, as they're usually running special events or just chatting with players. What fun!

A GM can only be detected if they choose to be detectable. If a GM wants to be undetected, **this plugin will respect their privacy.**
<!--desc-end-->

## Commands

<a href="cmd-gmcheck/">
{% 
  include-markdown "projects/mq2gmcheck/cmd-gmcheck.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2gmcheck/cmd-gmcheck.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2gmcheck/cmd-gmcheck.md') }}

## Settings

Example MQ2GMCheck.ini. If you'd like an audio alert, you should manually create this file and add a location under [Audio]
 

```ini
[Audio]
Sound=C:\\Users\\Redbot\\Games\\mqueue\\resources\\sounds\\gmcheck.wav
[GM]
Miserina=1,vox,Wed Oct 17 21:08:23 2018
Reelzl=1,vox,Thu Jan  3 19:00:45 2019
Babbsa=1,vox,Thu Jan  3 19:04:32 2019
Iqbal=1,vox,Wed May 29 21:20:56 2019
Sparrowz=1,vox,Thu May 30 23:01:19 2019
Simun=1,vox,Thu Aug 29 22:59:29 2019
[vox-The Plane of Knowledge]
Miserina=1,Wed Oct 17 21:08:23 2018
Reelzl=1,Thu Jan  3 19:00:45 2019
Babbsa=1,Thu Jan  3 19:04:32 2019
Iqbal=1,Wed May 29 21:20:56 2019
Sparrowz=1,Thu May 30 23:01:19 2019
Simun=1,Thu Aug 29 22:59:29 2019
```

## Top-Level Objects

## [GMCheck](tlo-gmcheck.md)
{% include-markdown "projects/mq2gmcheck/tlo-gmcheck.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2gmcheck/tlo-gmcheck.md') }}

## DataTypes

## [GMCheck](datatype-gmcheck.md)
{% include-markdown "projects/mq2gmcheck/datatype-gmcheck.md" start="<!--dt-desc-start-->" end="<!--dt-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2gmcheck/datatype-gmcheck.md') }}

<h2>Members</h2>
{% include-markdown "projects/mq2gmcheck/datatype-gmcheck.md" start="<!--dt-members-start-->" end="<!--dt-members-end-->" %}
{% include-markdown "projects/mq2gmcheck/datatype-gmcheck.md" start="<!--dt-linkrefs-start-->" end="<!--dt-linkrefs-end-->" %}
