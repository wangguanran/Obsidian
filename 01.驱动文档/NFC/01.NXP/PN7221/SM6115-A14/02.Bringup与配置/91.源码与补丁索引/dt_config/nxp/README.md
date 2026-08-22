# dt_config/nxp 更名记录

匿名化处理（2026-08-22）：本目录部分文件原名含项目代号，已更名（旧名 → 新名映射见源码树 patch 引用，此处不保留明文）。关联 patch：0001-PN722x、0003-nfc-pn7221。

- `bengal-nfc-[项目代号].dtsi` → `bengal-nfc.dtsi`
- `bengal-nfc-idp-[项目代号].dts` → `bengal-nfc-idp-custom.dts`
- `bengal-nfc-pinctrl-[项目代号].dtsi` → `bengal-nfc-pinctrl-custom.dtsi`
- 核查提示：patch 中的旧文件名经「去项目代号」归一化后可匹配到新文件。

_Author: wangguanran_