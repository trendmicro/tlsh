###########################
# see the malbaz.ipynb
#	Mirai / Gafgyt
#	in that notebook we see that Mirai and Gafgyt share code and get mis-labelled for each other
#	https://securityaffairs.co/wordpress/116882/cyber-crime/gafgyt-re-uses-mirai-code.html
#	We therefore put them in the MiraiGafgyt group
#
#	AgentTesla / SnakeKeylogger / Formbook / Loki
#	The notebook highlights that these malware families are difficult to distinguish
#	https://cyberintelmag.com/malware-viruses/year-long-spear-phishing-campaign-targets-energy-sector-with-agent-tesla-other-rats/
#	and get mis-lablled as each other
#	We therefore put them in the TeslaGroup
###########################
s/^Mirai$/MiraiGafgyt/
s/^Gafgyt$/MiraiGafgyt/
s/^AgentTesla$/TeslaGroup/
s/^SnakeKeylogger$/TeslaGroup/
s/^Formbook$/TeslaGroup/
s/^Loki$/TeslaGroup/
