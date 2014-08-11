yubiswitch-for-linux
====================

Simple program to enable/disable Yubikeys on Linux

To build and install:
---------------------

```
$ gcc -g -Wall -o yubiswitch main.c
# install -o root  -g root -m 4755  yubiswitch /usr/local/bin/
```

(yes, unfortunately it needs to be setuid root)


Usage
-----

```
$ yubiswitch
Usage: yubiswitch {on|off|list|debug}

$ yubiswitch list
Found Yubikey at 1-2:1.0
Yubikey is not bound to usbhid

$ yubiswitch on
binding Yubikey to usbhid driver at 1-2:1.0

$ yubiswitch off
Unbinding Yubikey from usbhid driver at 1-2:1.0
```


Integrations
-----------

If you use a old-fashioned window manager you can usually use it to
bind a key to run commands. For example for i3 I use:

```
bindsym $mod+y exec "/usr/local/bin/yubiswitch on"
bindsym $mod+Shift+y exec "/usr/local/bin/yubiswitch off"
```

For Heroku there's a nice plugin that monkeypatches the 2FA
implementation to call yubiswitch as needed at:

https://github.com/will/heroku-enable-yubikey-only-when-it-is-needed

If you use Duo and you run your shells inside Emacs you can add a
filter like the following to automatically turn the Yubikey on and off
whenever Duo presents a login prompt. This depends on your Duo
integration however. And it doesn't work in XTerms.

I've looked for a Duo API for their Duo Push which would allow me to
implement a demon which listened for Duo login attempts anywhere and
enabled the Yubikey as needed. However this API does not appear to be
documented amongst the Duo APIs.

```
(defvar duo-password-prompt-regexp "Duo two-factor login for")
(defvar duo-password-success-regexp "Success. Logging you in...")

(defun watch-for-duo-prompt (string)
  "Enable Yubikey as needed for Duo logins

  This function could be in the list `comint-output-filter-functions'."
  (when (string-match duo-password-prompt-regexp string)
	(call-process "yubiswitch" nil 0 nil "on"))
  (when (string-match duo-password-success-regexp string)
	(call-process "yubiswitch" nil 0 nil "off"))
  )
```



Todo
----

Perhaps an option to turn the yubikey on for a fixed length of time
then off when that time expires. 

Or to watch the yubikey and turn it off immediately after it's used?

Perhaps a deamon option to automatically turn it on when a message is
received and off after a timeout?

