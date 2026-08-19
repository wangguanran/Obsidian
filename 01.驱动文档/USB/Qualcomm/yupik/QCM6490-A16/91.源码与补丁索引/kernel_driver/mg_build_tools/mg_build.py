#!/usr/bin/python3

import os
import sys
import argparse
import subprocess
from datetime import datetime

MY_DIR = os.path.dirname(os.path.abspath(__file__))

PROJECT_NAME = "MC938_LINUX"
AP_GIT_VERSION = subprocess.check_output(['git', 'log', '--pretty=%h'], text=True).splitlines()[0].strip().upper()
COMPILE_DATE = datetime.now().strftime("%Y%m%d")
INTERN_VERSION="V01"
VERSION="01"
VER_DIR=f'{PROJECT_NAME}_{AP_GIT_VERSION}_{COMPILE_DATE}_{INTERN_VERSION}_{VERSION}'
SMBL_DIR=f'{PROJECT_NAME}_{AP_GIT_VERSION}_{COMPILE_DATE}_{INTERN_VERSION}_{VERSION}_sysmbol'


SUB_SYS_BUILD_RULES = {
    "SECTOOL":{
        "alias" : ["SECTOOL", "SECTOOLS"],
        "depends": [],
        "cmd": "",
        "export": """
        export SECTOOLS="${MG_WORK_ROOT}/${MG_PRODUCT}/common/sectoolsv2/ext/Linux/sectools"
        export SECTOOLS_DIR="${MG_WORK_ROOT}/${MG_PRODUCT}/common/sectoolsv2/ext/Linux"
        """
    },

    "PYTHON": {
        "alias" : ["PYTHON.3.10.2", "PYTHON", "PY"],
        "depends": [],
        "cmd": """
        if [ ! -e "${MG_WORK_ROOT}/mg_build_tools/python-3.10.2/bin/python" ]; then
            if cd  "${MG_WORK_ROOT}/mg_build_tools" ; then
                tar -xzvf Python-3.10.2.tgz
                cd Python-3.10.2
                ./configure --prefix="${MG_WORK_ROOT}/mg_build_tools/python-3.10.2"
                make -j16
                make install
                ln -snf python3.10 "${MG_WORK_ROOT}/mg_build_tools/python-3.10.2/bin/python"
                "${MG_WORK_ROOT}/mg_build_tools/python-3.10.2/bin/python" -m pip install --no-index --find-links  "${MG_WORK_ROOT}/mg_build_tools/pip_pkgs/" -r "${MG_WORK_ROOT}/mg_build_tools/pip_pkgs/requirements.txt"
            fi
        fi
        """,
        "export": """
        export PATH="${MG_WORK_ROOT}/mg_build_tools/python-3.10.2/bin:$PATH"
        export PYTHONPATH="${MG_WORK_ROOT}/mg_build_tools/python-3.10.2/lib:$PYTHONPATH"
        """
    },

    "ADSP": {
        "alias" : ["ADSP.HT.5.5.C8", "ADSP"],
        "depends": ["SECTOOL", "PYTHON"],
        "cmd": """
        if cd "${MG_WORK_ROOT}/ADSP.HT.5.5.c8/adsp_proc/build/ms" ; then
            python ./build_variant.py kodiak.adsp.prod
        fi

        cp ${MG_WORK_ROOT}/ADSP.HT.5.5.c8/adsp_proc/*.elf ${MG_WORK_ROOT}/out/${SYSMBOL_DIR}/
        cp ${MG_WORK_ROOT}/ADSP.HT.5.5.c8/adsp_proc/build/ms/*.elf ${MG_WORK_ROOT}/out/${SYSMBOL_DIR}/
        cp ${MG_WORK_ROOT}/ADSP.HT.5.5.c8/adsp_proc/qdsp6/qshrink/src/msg_hash.txt ${MG_WORK_ROOT}/out/${SYSMBOL_DIR}/

        """,
        "export": ""
    },

    "AOP": {
        "alias" : ["AOP.HO.3.6", "AOP"],
        "depends": ["SECTOOL", "PYTHON"],
        "cmd": """
        if cd "${MG_WORK_ROOT}/AOP.HO.3.6" ; then
            bash "./aop_proc/build/build_kodiak.sh" -l /pkg/qct/software/llvm/release/arm/14.0.4/
        fi
        echo "xxxxxxx"
        echo ${VERSION_DIR}
        echo ${SYSMBOL_DIR}
        cp ${MG_WORK_ROOT}/AOP.HO.3.6/aop_proc/build/ms/bin/AAAAANAZO/kodiak/aop.mbn ${MG_WORK_ROOT}/out/${VERSION_DIR}/

        cp ${MG_WORK_ROOT}/AOP.HO.3.6/aop_proc/core/bsp/aop/build/kodiak/*.elf ${MG_WORK_ROOT}/out/${SYSMBOL_DIR}/
	
		""",
        "export": ""
    },

    "BOOT": {
        "alias" : ["BOOT.MXF.1.0.C1", "BOOT", "BOOTIMAGE"],
        "depends": ["SECTOOL", "PYTHON"],
        "cmd": """
        cp ${MG_WORK_ROOT}/PROJECT/odm_features.h ${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Include/
        python -u "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot_tools/buildex.py" -v WP -t kodiak,QcomToolsPkg -v LAA -r RELEASE

        source_files=(
                    "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Bin/LAA/RELEASE/xbl.elf"
                    "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Bin/LAA/RELEASE/xbl_config.elf"
                    "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Bin/LAA/RELEASE/XblRamdump.elf"
                    "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Bin/LAA/RELEASE/uefi.elf"
                    "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Bin/LAA/RELEASE/shrm.elf"
                    "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Bin/LAA/RELEASE/imagefv.elf"
                    "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/QcomToolsPkg/Bin/QcomTools/RELEASE/tools.fv"
                    "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Bin/LAA/RELEASE/prog_firehose_ddr.elf"
            )
        
            target_dir="${MG_WORK_ROOT}/out/${VERSION_DIR}"

            for file in "${source_files[@]}"; do
                if [ -f "$file" ] || [ -d "$file" ]; then
                    cp -rf "$file" "$target_dir"
                else
                    echo "$file not exist."
                    exit 1
                fi
            done

        cp ${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/Build/KodiakLAA/Loader/RELEASE_CLANG140LINUX/AARCH64/QcomPkg/XBLLoader/XBLLoader/DEBUG/XBLLoader.dll ${MG_WORK_ROOT}/out/${SYSMBOL_DIR}/
        """,
        "export": ""
    },

    "CDSP": {
        "alias" : ["CDSP.HT.2.5.C3", "CDSP"],
        "depends": ["SECTOOL", "PYTHON"],
        "cmd": """
        if cd "${MG_WORK_ROOT}/CDSP.HT.2.5.c3/cdsp_proc/build/ms" ; then
            python ./build_variant.py kodiak.cdsp.prod
        fi
		
        source_files=(
                    $(ls ${MG_WORK_ROOT}/CDSP.HT.2.5.c3/cdsp_proc/*.elf)
                    $(ls ${MG_WORK_ROOT}/CDSP.HT.2.5.c3/cdsp_proc/build/ms/*.elf)
                    "${MG_WORK_ROOT}/CDSP.HT.2.5.c3/cdsp_proc/qdsp6/qshrink/src/msg_hash.txt"
            )
        
            target_dir="${MG_WORK_ROOT}/out/${SYSMBOL_DIR}/"

            for file in "${source_files[@]}"; do
                if [ -f "$file" ] || [ -d "$file" ]; then
                    cp -rf "$file" "$target_dir"
                else
                    echo "$file not exist."
                    exit 1
                fi
            done

        """,
        "export": ""
    },

    "TZ": {
        "alias" : ["TZ.XF.5.29.1", "TZ", "TRUSTZONE"],
        "depends": ["SECTOOL", "PYTHON"],
        "cmd": """
        cp ${MG_WORK_ROOT}/PROJECT/odm_features.h  ${MG_WORK_ROOT}/TZ.XF.5.29.1/trustzone_images/build/ms/
        if cd "${MG_WORK_ROOT}/TZ.XF.5.29.1/trustzone_images/build/ms" ; then
            python build_all.py -b TZ.XF.5.0 CHIPSET=kodiak --cfg=build_config_deploy_kodiak.xml
        fi
        
        source_files=(
                    "${MG_WORK_ROOT}/TZ.XF.5.29.1/trustzone_images/build/ms/bin/EACAANAA/tz.mbn"
                    "${MG_WORK_ROOT}/TZ.XF.5.29.1/trustzone_images/build/ms/bin/EACAANAA/hypvm.mbn"
                    "${MG_WORK_ROOT}/TZ.XF.5.29.1/trustzone_images/build/ms/bin/EACAANAA/devcfg.mbn"
                    "${MG_WORK_ROOT}/TZ.APPS.1.29/qtee_tas/build/ms/bin/IAGAANAA/uefi_sec.mbn"
            )
        
            target_dir="${MG_WORK_ROOT}/out/${VERSION_DIR}"

            for file in "${source_files[@]}"; do
                if [ -f "$file" ] || [ -d "$file" ]; then
                    cp -rf "$file" "$target_dir"
                else
                    echo "$file not exist."
                    exit 1
                fi
            done

        cp ${MG_WORK_ROOT}/TZ.XF.5.29.1/trustzone_images/ssg/bsp/qsee/build/EACAANAA/qsee.elf ${MG_WORK_ROOT}/out/${SYSMBOL_DIR}/
		""",
        "export": ""
    },

    "AP": {
        "alias" : ["LE.QCLINUX.1.0.R1", "LE", "LINUX", "AP", "SYSTEM", "SYSTEMIMAGE"],
        "depends": ["SECTOOL", "PYTHON"],
        "cmd": """
        # export BB_NO_NETWORK=1
        export MACHINE=qcm6490-idp
        export DISTRO=qcom-wayland
        export EXTRALAYERS=\"meta-qcom-qim-product-sdk meta-qcom-extras\"
        export FWZIP_PATH=${MG_WORK_ROOT}/QCM6490.LE.1.0/common/build/ufs/bin
        export CUST_ID=\"213195\"

        if cd "${MG_WORK_ROOT}/LE.QCLINUX.1.0.r1" ; then
            if [ ! -e ./downloads ]; then
                curl --retry 15 --retry-delay 30  -C -  -O  -u linux_build:FVPGG4kEx1rafexS  ftp://116.247.69.94:1121/downloads/qcs6490/qcom6490_1_6_downloads.tar.gz
                tar -xzvf  qcom6490_1_6_downloads.tar.gz
            fi
            if [ "${BUILD_TYPE}" == "debug" ]; then
                echo "build debug version"
                DEBUG_BUILD=1 source setup-environment
                DEBUG_BUILD=1 bitbake qcom-multimedia-image
                DEBUG_BUILD=1 bitbake qcom-qim-product-sdk
                
            elif [ "${BUILD_TYPE}" == "performance" ]; then
                echo "build perf version"
                PERFORMANCE_BUILD=1 source setup-environment
                PERFORMANCE_BUILD=1 bitbake qcom-multimedia-image
                PERFORMANCE_BUILD=1 bitbake qcom-qim-product-sdk
            else
                echo "default build normal version"
                source setup-environment
                bitbake qcom-multimedia-image
                bitbake qcom-qim-product-sdk
            fi

            source_files=(
                    "${MG_WORK_ROOT}/LE.QCLINUX.1.0.r1/build-qcom-wayland/tmp-glibc/deploy/images/qcm6490-idp/qcom-multimedia-image/system.img"
                    "${MG_WORK_ROOT}/LE.QCLINUX.1.0.r1/build-qcom-wayland/tmp-glibc/deploy/images/qcm6490-idp/qcom-multimedia-image/dtb.bin"
                    "${MG_WORK_ROOT}/LE.QCLINUX.1.0.r1/build-qcom-wayland/tmp-glibc/deploy/images/qcm6490-idp/qcom-multimedia-image/efi.bin"
                    "${MG_WORK_ROOT}/LE.QCLINUX.1.0.r1/build-qcom-wayland/tmp-glibc/deploy/images/qcm6490-idp/qcom-multimedia-image/vmlinux"
                    "${MG_WORK_ROOT}/LE.QCLINUX.1.0.r1/build-qcom-wayland/qim-prod-sdk"
                    "${MG_WORK_ROOT}/LE.QCLINUX.1.0.r1/build-qcom-wayland/qim-sdk"
                    "${MG_WORK_ROOT}/LE.QCLINUX.1.0.r1/build-qcom-wayland/tflite-sdk"
            )
        
            target_dir="${MG_WORK_ROOT}/out/${VERSION_DIR}"

            for file in "${source_files[@]}"; do
                if [ -f "$file" ] || [ -d "$file" ]; then
                    cp -rf "$file" "$target_dir"
                else
                    echo "$file not exist."
                    exit 1
                fi
            done

        fi
        """,
        "export": ""
    },

    "BP": {
        "alias" : ["BSP"],
        "depends": ["ADSP", "AOP", "BOOT", "CDSP", "TZ"],
        "cmd": """
        if cd "${MG_WORK_ROOT}/${MG_PRODUCT}/common/build" ; then
            python3 build.py --imf

            source_files=(
                    $(ls ${MG_WORK_ROOT}/${MG_PRODUCT}/common/build/ufs/zeros_*.bin)
                    $(ls ${MG_WORK_ROOT}/${MG_PRODUCT}/common/build/ufs/gpt_*.bin)
                    "${MG_WORK_ROOT}/${MG_PRODUCT}/common/build/Ver_Info.txt"
                    "${MG_WORK_ROOT}/${MG_PRODUCT}/contents.xml"
                    "${MG_WORK_ROOT}/${MG_PRODUCT}/common/build/bin/multi_image.mbn"
                    "${MG_WORK_ROOT}/${MG_PRODUCT}/common/core_qupv3fw/kodiak/qupv3fw.elf"
                    "${MG_WORK_ROOT}/CPUCP.FW.1.0/cpucp_proc/kodiak/cpucp/cpucp.elf"
                    "${MG_WORK_ROOT}/${MG_PRODUCT}/common/config/ufs/ext/partition_ufs.xml"
                    $(ls ${MG_WORK_ROOT}/${MG_PRODUCT}/common/build/ufs/*.xml)
            )
        
            target_dir="${MG_WORK_ROOT}/out/${VERSION_DIR}"

            for file in "${source_files[@]}"; do
                if [ -f "$file" ] || [ -d "$file" ]; then
                    cp -rf "$file" "$target_dir"
                else
                    echo "$file not exist."
                    exit 1
                fi
            done

		fi
        """,
        "export": ""
    },

    "ALL": {
        "alias" : ["ALL","IMAGE","IMG","PACK","PACKAGE"],
        "depends": ["BP", "AP"],
        "cmd": """
        echo "All Done with Success!"
        """,
        "export": ""
    },
}


def build_sub_sys(sub_sys, product ,version, type):
    if sub_sys not in SUB_SYS_BUILD_RULES:
        print(f"unknown build sub sys:{sub_sys}")
        return
    sub_sys_rule = SUB_SYS_BUILD_RULES[sub_sys]
    depend_export = ""
    for depend in sub_sys_rule["depends"]:
        if depend in SUB_SYS_BUILD_RULES:
            build_sub_sys(depend, product ,version, type)
            depend_export += SUB_SYS_BUILD_RULES[depend]["export"]

    tmp_sh = ".tmp.sh"
    print("sub_sys=", sub_sys)
    print("product=", product)
    print("version=", version)
    cmd = "#!/bin/bash\n"
    cmd += f"export MG_WORK_ROOT={MY_DIR}\n"
    cmd += f"export MG_PRODUCT={product}\n"
    cmd += f"BUILD_TYPE={type}\n"
    cmd += f"export VERSION_DIR={VER_DIR}\n"
    cmd += f"export SYSMBOL_DIR={SMBL_DIR}\n"
    if len(depend_export) > 0:
        cmd += f"{depend_export}\n"
    cmd += f"{sub_sys_rule['cmd']}\n"

    print(f"Executing command: {cmd}")

    with open(tmp_sh, "wt+", encoding="utf-8", errors="ignore") as f:
        f.write( cmd )
        f.close()
    ret = os.system("bash .tmp.sh")
    os.unlink(tmp_sh)
    if ret != 0:
        print(f"Done with error: {ret}")
        exit(ret)
    print(f"Done with success!")


def packagefiles():
    zip_filename = f'{PROJECT_NAME}_{AP_GIT_VERSION}_{COMPILE_DATE}_{INTERN_VERSION}_{VERSION}.zip'
    pack_sh = ".pack.sh"
    cmd = "#!/bin/bash\n"
    cmd += f"cd {MY_DIR}/out/\n"
    cmd += f"zip -r {zip_filename} {VER_DIR}\n"
    print(f"Executing command: {cmd}")
    with open(pack_sh, "wt+", encoding="utf-8", errors="ignore") as f:
        f.write( cmd )
        f.close()
    ret = os.system("bash .pack.sh")
    os.unlink(pack_sh)
    if ret != 0:
        print(f"Packaged Done with error: {ret}")
        exit(ret)
    print(f'Packaged into {zip_filename}')


def parse_arg():
    parser = argparse.ArgumentParser(description="MeiG Copmpile Script")
    parser.add_argument('-p', "--product", type=str, choices=['QCM6490.LE.1.0', 'QCS9100.LE.1.0'], default='QCM6490.LE.1.0', help='Product id to build')
    parser.add_argument('-l', "--list", action='store_true', help='list available builds')
    parser.add_argument('-v', "--version", type=str, help='version num for builds', default="01")
    parser.add_argument('-t', "--type", type=str, choices=['debug', 'performance', 'normal'], default="normal", help='build type debug/performance/normal version')
    return parser.parse_known_args()


def create_output_dir():
    path1 = "{}/out/{}".format(os.getcwd(), VER_DIR)
    path2 = "{}/out/{}".format(os.getcwd(), SMBL_DIR)
    os.makedirs(path1, exist_ok=True);
    os.makedirs(path2, exist_ok=True);

def main():
    args, unknown = parse_arg()
    if args.list:
        for key in SUB_SYS_BUILD_RULES.keys():
            print( key )
        return

    # create output dir
    create_output_dir()

    # process args to targets
    targets = []
    for arg in unknown:
        arg = arg.upper()
        for key in SUB_SYS_BUILD_RULES:
            if arg == key.upper() or arg in SUB_SYS_BUILD_RULES[key]["alias"]:
                targets.append(key)
                break

    # process depends
    for target in targets:
        try:
            build_sub_sys(target, args.product, args.version, args.type)
        except Exception as e:
            print(f"================Error building subsystem {target}: {e}======================")
            return
    
    if 'ALL' in targets:
        packagefiles()


if __name__ == "__main__":
    main()
