# MediaTek MT7902 WiFi 드라이버 빌드 및 수동 설치 가이드 (Ubuntu / Linux)

이 문서는 MediaTek MT7902 (PCI ID: `14c3:7902`) 칩셋을 사용하는 모델(예: ASUS Vivobook Go 등)에서 커널 업데이트 후 WiFi 드라이버가 풀렸을 때 최소한의 실패로 다시 빌드하고 설치하는 방법을 정리합니다.

> [!NOTE]
> MT7902는 아직 메인라인 커널(Kernel 6.17 기준)에 완벽하게 정식 지원되지 않아 호환성 문제가 생길 수 있습니다. 이 커스텀 빌드 방법은 문제를 피하기 위한 우회 방법(Workaround)입니다.

## 0. 검증된 환경 (Tested Environment)

이 가이드는 아래의 환경에서 성공적으로 테스트되었습니다. 비슷한 사양의 사용자들은 더 높은 확률로 성공할 수 있습니다.

*   **노트북 모델**: ASUS Vivobook Go E1504FA (E1504FA_E1504FA)
*   **운영체제**: Ubuntu 25.10 (64-bit)
*   **커널 버전**: `6.17.0-20-generic`
*   **BIOS 버전**: E1504FA.308 (2023-11-16)
*   **컴파일러**: gcc (Ubuntu 15.2.0-4ubuntu4) 15.2.0

---

## 1. 개요 및 사전 준비

### 발생 원인
- 기본 커널에 포함된 `mt7921e` 혹은 `mt7925e` 모듈이 `MT7902` 칩을 잡긴 하지만, 펌웨어 통신(patch semaphore)에 실패하면서 하드웨어 초기화에 실패합니다 (`dmesg` 로그: `Failed to get patch semaphore`).
- 이를 해결하기 위해 커스텀 패치가 적용된 소스를 직접 빌드하여, 기존 드라이버 대신 로드해야 합니다.

### 필수 패키지 설치 (최초 1회 혹은 재설치 시 필요)
인터넷 연결이 필요합니다 (LAN 케이블, 혹은 스마트폰 USB 테더링 활용).
```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r) bc
```

---

## 2. 드라이버 빌드 과정

### 2.1 저장소 준비
커뮤니티 패치 버전인 `mt7902_temp` 저장소를 다운로드합니다. (현재 가이드는 이 저장소에 포함된 자동화 패치를 기준으로 합니다.)
```bash
cd ~/dev
git clone --depth 1 https://github.com/OnlineLearningTutorials/mt7902_temp
cd mt7902_temp/latest
```

### 2.2 빌드 준비 (자동화 패치 포함)
이 저장소에는 최신 커널(6.17+)에서 발생하는 `airoha_offload.h` 누락 에러를 해결하기 위한 **자동화 패치**가 이미 포함되어 있습니다. 별도의 수동 헤더 파일 생성 없이 바로 빌드가 가능합니다.

---

### 2.3 드라이버 모듈 컴파일
준비가 끝나면 클린 후 빌드를 진행합니다.
준비가 끝나면 클린 후 빌드를 진행합니다.
```bash
cd ~/dev/mt7902_temp/latest
make clean
make module_compile
```
> 성공 시 에러 없이 끝나며 디렉토리에 `.ko` 파일 5개(`mt76.ko`, `mt76-connac-lib.ko`, `mt792x-lib.ko`, `mt7921/mt7921-common.ko`, `mt7921/mt7921e.ko`)가 생성되어야 합니다.

---

## 3. 커스텀 모듈 및 펌웨어 설치

### 3.1 커스텀 폴더에 모듈 복사
기존 시스템 드라이버와 섞이지 않도록 `/lib/modules/mt7902_custom/` 에 따로 저장합니다.
```bash
sudo mkdir -p /lib/modules/mt7902_custom/
sudo cp ~/dev/mt7902_temp/latest/mt76.ko /lib/modules/mt7902_custom/
sudo cp ~/dev/mt7902_temp/latest/mt76-connac-lib.ko /lib/modules/mt7902_custom/
sudo cp ~/dev/mt7902_temp/latest/mt792x-lib.ko /lib/modules/mt7902_custom/
sudo cp ~/dev/mt7902_temp/latest/mt7921/mt7921-common.ko /lib/modules/mt7902_custom/
sudo cp ~/dev/mt7902_temp/latest/mt7921/mt7921e.ko /lib/modules/mt7902_custom/
```

### 3.2 펌웨어 복사
Mediatek MT7902 와이파이 / 블루투스 구동을 위한 바이너리 파일들을 커널 펌웨어 디렉토리에 복사합니다.
```bash
cd ~/dev/mt7902_temp/mt7902_firmware/latest
sudo cp WIFI_MT7902_patch_mcu_1_1_hdr.bin /lib/firmware/mediatek/
sudo cp WIFI_RAM_CODE_MT7902_1.bin /lib/firmware/mediatek/
sudo cp BT_RAM_CODE_MT7902_1_1_hdr.bin /lib/firmware/mediatek/
```

---

## 4. 재부팅 지속성(Persistence) 확보 설정

기본 드라이버 로드를 막고 전용 서비스로 우리가 빌드한 모듈을 강제 로드하도록 설정해야 합니다.

### 4.1 기본 충돌 모듈 억제 (Blacklist)
커널이 부팅 시 내장된 (작동하지 않는) mt7925e 따위를 잡지 못하도록 블랙리스트를 구성합니다.

```bash
sudo tee /etc/modprobe.d/mt7902-blacklist.conf > /dev/null << 'EOF'
# MT7902 커스텀 드라이버를 위해 기본 모듈 자동 로드 차단
blacklist mt7925e
blacklist mt7925_common
EOF

# 적용 (반드시 해줘야 초기 부팅 시 불려오지 않음)
sudo update-initramfs -u
```

### 4.2 모듈 자동 로드 스크립트 작성
```bash
sudo tee /usr/local/bin/mt7902-setup.sh > /dev/null << 'EOF'
#!/bin/bash
# Unload conflicting modules
rmmod btusb btmtk mt7925e mt7925_common mt7921e mt7921_common mt792x_lib mt76_connac_lib mt76 2>/dev/null

modprobe cfg80211
modprobe mac80211

insmod /lib/modules/mt7902_custom/mt76.ko
insmod /lib/modules/mt7902_custom/mt76-connac-lib.ko
insmod /lib/modules/mt7902_custom/mt792x-lib.ko
insmod /lib/modules/mt7902_custom/mt7921-common.ko
insmod /lib/modules/mt7902_custom/mt7921e.ko
EOF

sudo chmod +x /usr/local/bin/mt7902-setup.sh
```

### 4.3 systemd 서비스 등록
네트워크가 시작되기 전 이 쉘스크립트를 실행하여 모듈을 깔아두도록 합니다.
```bash
sudo tee /etc/systemd/system/mt7902.service > /dev/null << 'EOF'
[Unit]
Description=Load custom MT7902 Wi-Fi drivers
After=network-pre.target
Before=network.target
Wants=network-pre.target

[Service]
Type=oneshot
ExecStart=/usr/local/bin/mt7902-setup.sh
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable mt7902.service
```

---

## 5. 커널 업데이트 후 재적용

리눅스 커널 패키지가 판올림(예: `6.17.0-20` -> `6.17.0-21`)되면 기존에 컴파일한 `.ko` 모듈들이 새 커널 버전과 안 맞아서 로드가 안 되게 됩니다. 그때는 다음 명령만 수행하면 됩니다.

1. 재부팅 후 테더링 등으로 인터넷 연결
2. `sudo apt install linux-headers-$(uname -r)`
3. 위 문서의 **2.2 빌드 에러 조치** (새 커널 헤더 폴더에 airoha_offload.h 스텁 복사) 부분 실행
4. `cd ~/dev/mt7902_temp/latest`
5. `make clean && make module_compile`
6. 생성된 5개 `.ko` 파일들을 `sudo cp` 로 `/lib/modules/mt7902_custom/` 에 덮어쓰기
7. 스크립트 실행 테스트: `sudo /usr/local/bin/mt7902-setup.sh`
8. 완료되면 재부팅하거나 즉시 와이파이 사용 가능.
