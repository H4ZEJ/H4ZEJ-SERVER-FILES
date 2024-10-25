import app
import localeInfo
from constInfo import TextColor
app.ServerName = None

SRV1 = {
	"name":TextColor("ServerName", "ffd500"), #GOLD
	"host":"127.0.0.1",
	"auth1":30001,
	"ch1":30003,
	"ch2":30007, #if you only have 1 ch and see it online, it's ch99 having the same port
	"ch3":30011,
	"ch4":30015,
}

STATE_NONE = TextColor(localeInfo.CHANNEL_STATUS_OFFLINE, "FF0000") #RED

STATE_DICT = {
	0: TextColor(localeInfo.CHANNEL_STATUS_OFFLINE, "FF0000"), #RED
	1: TextColor(localeInfo.CHANNEL_STATUS_RECOMMENDED, "00ff00"), #GREEN
	2: TextColor(localeInfo.CHANNEL_STATUS_BUSY, "ffff00"), #YELLOW
	3: TextColor(localeInfo.CHANNEL_STATUS_FULL, "ff8a08") #ORANGE
}

SERVER1_CHANNEL_DICT = {
	0: {"key":10, "name":TextColor("CH-1", "FFffFF"), "ip":SRV1["host"], "tcp_port":SRV1["ch1"], "udp_port":SRV1["ch1"], "state":STATE_NONE,},
	1: {"key":11, "name":TextColor("CH-2", "FFffFF"), "ip":SRV1["host"], "tcp_port":SRV1["ch2"], "udp_port":SRV1["ch2"], "state":STATE_NONE,},
	2: {"key":12, "name":TextColor("CH-3", "FFffFF"), "ip":SRV1["host"], "tcp_port":SRV1["ch3"], "udp_port":SRV1["ch3"], "state":STATE_NONE,},
	3: {"key":13, "name":TextColor("CH-4", "FFffFF"), "ip":SRV1["host"], "tcp_port":SRV1["ch4"], "udp_port":SRV1["ch4"], "state":STATE_NONE,},
}

REGION_NAME_DICT = {
	0: SRV1["name"],
}

REGION_AUTH_SERVER_DICT = {
	0: {
		0: {"ip": SRV1["host"], "port": SRV1["auth1"],},
		1: {"ip": SRV1["host"], "port": SRV1["auth1"],},
		2: {"ip": SRV1["host"], "port": SRV1["auth1"],},
		3: {"ip": SRV1["host"], "port": SRV1["auth1"],},
	}
}

REGION_DICT = {
	0: {
		1: {"name": SRV1["name"], "channel": SERVER1_CHANNEL_DICT,},
	},
}

MARKADDR_DICT = {
	10: {"ip": SRV1["host"], "tcp_port": SRV1["ch1"], "mark": "10.tga", "symbol_path": "10",},
}

TESTADDR = {"ip": SRV1["host"], "tcp_port": SRV1["ch1"], "udp_port": SRV1["ch1"],}
