# Hedgehog-proxy utility. 
It`s a lightweight proxy utility, that was design to be used in the Hyprland waybar.

### Install 
Simply copy and paste this commands in your terminal
```
git clone https://github.com/reebuker/hedgehog-proxy.git
cd hedgehog-proxy
make setup #This will require root to place hedgehog binary in /usr local/bin/  
```

### Waybar config
Locate your waybar config, usually at
~/.config/waybar/config.jsonc \
Paste this lines among other modules
##### config.jsonc
```
  "custom/hedgehog": {
    "exec": "hedgehog status",
    "interval": 3,
    "return-type": "json",
    "on-click": "hedgehog toggle",
    "on-click-right": "hedgehog-gui"
  },
```
Don't forget to place it in one of your modules, for example:
```
  "modules-right": [
    "custom/hedgehog",
  ],
```

You can also locate your styles file, usually at 
~/.config/waybar/style.css
##### style.css
```
#custom-hedgehog {
    margin: 0 4px;
    font-size: 1.3rem;
}

#custom-hedgehog.hedgehog-off { color: red }
#custom-hedgehog.hedgehog-on { color: green }
```
