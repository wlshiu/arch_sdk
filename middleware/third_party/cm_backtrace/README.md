# CmBacktrace: ARM Cortex-M 系列 MCU 錯誤追蹤庫

[![GitHub release](https://img.shields.io/github/release/armink/CmBacktrace.svg)](https://github.com/armink/CmBacktrace/releases/latest) [![GitHub commits](https://img.shields.io/github/commits-since/armink/CmBacktrace/1.3.0.svg)](https://github.com/armink/CmBacktrace/compare/1.0.0...master) [![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/armink/CmBacktrace/master/LICENSE)

## 0、CmBacktrace 是什麼

[CmBacktrace](https://github.com/armink/CmBacktrace) （Cortex Microcontroller Backtrace）是一款針對 ARM Cortex-M 系列 MCU 的錯誤程式碼自動追蹤、定位，錯誤原因自動分析的開源庫。主要特性如下：

- 支援的錯誤包括：
    - 斷言（assert）
    - 故障（Hard Fault, Memory Management Fault, Bus Fault, Usage Fault, Debug Fault）
- 故障原因 **自動診斷** ：可在故障發生時，自動分析出故障的原因，定位發生故障的程式碼位置，而無需再手動分析繁雜的故障暫存器；
- 輸出錯誤現場的 **函式呼叫棧**（需配合 addr2line 工具進行精確定位），還原發生錯誤時的現場資訊，定位問題程式碼位置、邏輯更加快捷、精準。也可以在正常狀態下使用該庫，獲取目前的函式呼叫棧；
- 支援 裸機 及以下操作系統平臺：
    - [RT-Thread](http://www.rt-thread.org/)
    - UCOS
    - FreeRTOS（需修改原始碼）
- 根據錯誤現場狀態，輸出對應的 執行緒棧 或 C 主棧；
- 故障診斷資訊支援多國語言（目前：簡體中文、英文）；
- 適配 Cortex-M0/M3/M4/M7 MCU；
- 支援 IAR、KEIL、GCC 編譯器；

## 1、為什麼選擇 CmBacktrace

**入門新人** ：對於從 C51 、MSP430 等簡單微控制器轉而使用更加複雜的 ARM  新人來說，時不時出現的 "hard falut" 宕機會讓新人瞬間懵掉。定位錯誤的方法也往往是連線上模擬器，一步步 F10/F11 單步，定位到具體的錯誤程式碼，再去猜測、排除、推敲錯誤原因，這種過程十分痛苦。

**熟練老手** ：慢慢的大家知道可以通過故障暫存器資訊來定位故障原因及故障程式碼地址，雖然這樣能解決一小部分問題，但是重複的、繁瑣的分析過程也會耽誤很多時間。而且對於一些複雜問題，只依靠程式碼地址是無法解決的，必須得還原錯誤現場的函式呼叫邏輯關係。雖然連線模擬器可以檢視到的函式呼叫棧，但故障狀態下是無法顯示的，所以還是得一步步 F10/F11 單步去定位錯誤程式碼的位置。另外，還有兩種場景，

- 1、很多產品真機除錯時必須斷開模擬器
- 2、問題確實存在，但是極難被重現

所以定位這類問題就顯得難上加難。

**使用本庫** ：上述所有問題都迎刃而解，可以將錯誤資訊輸出到控制檯上，還可以將錯誤資訊使用 [EasyFlash](https://github.com/armink/EasyFlash) 的 Log 功能儲存至 Flash 中，裝置宕機後重啟依然能夠讀取上次的錯誤資訊。CmBacktrace 輸出的資訊包括函式呼叫棧、故障診斷結果、堆疊、故障暫存器及產品韌體資訊，極大的提升了錯誤定位的效率及準確性。

俗話說，工欲善其事，必先利其器。所以有時候做事效率低的原因也許是，你會用的工具種類太少。

**合作、貢獻** ：開源軟體的發展離不開大家的支援，歡迎大家多提建議，也希望更多的人一起參與進來，共同提高  。如果覺得這個開源專案很贊，可以點選 [專案主頁](https://github.com/armink/CmBacktrace) **([Github](https://github.com/armink/CmBacktrace)|[OSChina](http://git.oschina.net/armink/CmBacktrace)|[Coding](https://coding.net/u/armink/p/CmBacktrace/git))**  右上角的 **Star** ，同時把它推薦給更多有需要的朋友。

## 2、CmBacktrace 如何使用

### 2.1 演示

該演示分如下幾個步驟：

- 1、製造除零異常（[IAR 工程，點選檢視原始碼](https://github.com/armink/CmBacktrace/tree/master/demos/non_os/stm32f10x/app/src)）
- 2、檢視錯誤診斷資訊
- 3、檢視函式呼叫棧基本資訊
- 4、通過命令列工具進入專案工程存放可執行檔案的路徑
- 5、使用 addr2line 命令，檢視函式呼叫棧詳細資訊，並定位錯誤程式碼

[![cm_backtrace_demo](https://raw.githubusercontent.com/armink/CmBacktrace/master/docs/zh/images/cm_backtrace_demo.gif)](https://github.com/armink/CmBacktrace)

### 2.2 Demo

|目錄|平臺|鏈接|
|:--|:--:|:--:|
| `\demos\non_os\stm32f10x` |裸機 STM32 Cortex-M3|[點選檢視](https://github.com/armink/CmBacktrace/tree/master/demos/non_os/stm32f10x)|
| `\demos\os\rtthread\stm32f4xx`|RT-Thread STM32 Cortex-M4|[點選檢視](https://github.com/armink/CmBacktrace/tree/master/demos/os/rtthread/stm32f4xx)|
| `\demos\os\ucosii\stm32f10x` |UCOSII STM32 Cortex-M3|[點選檢視](https://github.com/armink/CmBacktrace/tree/master/demos/os/ucosii/stm32f10x)|
| `\demos\os\freertos\stm32f10x` |FreeRTOS STM32 Cortex-M3|[點選檢視](https://github.com/armink/CmBacktrace/tree/master/demos/os/freertos/stm32f10x)|



### 2.3 移植說明

#### 2.3.1 準備工作

- 1、檢視 `\demos` 目錄下有沒有合適自己的 Demo ，如有類似，則建議在其基礎上修改
- 2、明確操作系統/裸機平臺及 CPU 平臺
- 3、將 `\src` 下的全部原始檔新增至產品工程中，並保證原始碼目錄被新增至標頭檔案路徑
- 4、cmb_fault.s 彙編檔案（[點選檢視](https://github.com/armink/CmBacktrace/tree/master/cm_backtrace/fault_handler)）可以選擇性新增至工程，新增后需要把專案原有的 `HardFault_Handler` 註釋掉
- 5、把 `cm_backtrace_init` 函式放在專案初始化地方執行
- 6、將 `cm_backtrace_assert` 放在專案的斷言函式中執行，具體使用方法參照下面的 API 說明
- 7、如果第 4 步驟沒有將 cmb_fault.s 彙編檔案啟用，則需要將 `cm_backtrace_fault` 放到故障處理函式（例如： `HardFault_Handler` ）中執行，具體使用方法參照下面的 API 說明

#### 2.3.2 配置說明

配置檔名： `cmb_cfg.h` ，針對不同的平臺和場景，使用者需要自自行手動配置，常用配置如下：

| 配置名稱 |功能|備註|
|:--|:--|:--|
|cmb_println(...)|錯誤及診斷資訊輸出|必須配置|
|CMB_USING_BARE_METAL_PLATFORM|是否使用在裸機平臺|使用則定義該宏|
|CMB_USING_OS_PLATFORM|是否使用在操作系統平臺|操作系統與裸機必須二選一|
|CMB_OS_PLATFORM_TYPE|操作系統平臺|RTT/UCOSII/UCOSIII/FREERTOS|
|CMB_CPU_PLATFORM_TYPE|CPU平臺|M0/M3/M4/M7|
|CMB_USING_DUMP_STACK_INFO|是否使用 Dump 堆疊的功能|使用則定義該宏|
|CMB_PRINT_LANGUAGE|輸出資訊時的語言|CHINESE/ENGLISH|

> 注意：以上部分配置的內容可以在 `cmb_def.h` 中選擇，更多靈活的配置請閱讀原始碼

### 2.4 API 說明

#### 2.4.1 庫初始化

```C
void cm_backtrace_init(const char *firmware_name, const char *hardware_ver, const char *software_ver)
```

|參數                                    |描述|
|:-----                                  |:----|
|firmware_name                           |韌體名稱，需與編譯器產生的韌體名稱對應|
|hardware_ver                            |韌體對應的硬體版本號|
|software_ver                            |韌體的軟體版本號|

> **注意** ：以上入參將會在斷言或故障時輸出，主要起了追溯的作用

#### 2.4.2 獲取函式呼叫棧

```C
size_t cm_backtrace_call_stack(uint32_t *buffer, size_t size, uint32_t sp)
```

|參數                                    |描述|
|:-----                                  |:----|
|buffer                                  |儲存函式呼叫棧的緩衝區|
|size                                    |緩衝區大小|
|sp                                      |待獲取的堆疊指針|

示例：

```C
/* 建立深度為 16 的函式呼叫棧緩衝區，深度大小不應該超過 CMB_CALL_STACK_MAX_DEPTH（預設16） */
uint32_t call_stack[16] = {0};
size_t i, depth = 0;
/* 獲取目前環境下的函式呼叫棧，每個元素將會以 32 位地址形式儲存， depth 為函式呼叫棧實際深度 */
depth = cm_backtrace_call_stack(call_stack, sizeof(call_stack), cmb_get_sp());
/* 輸出目前函式呼叫棧資訊
 * 注意：檢視函式名稱及具體行號時，需要使用 addr2line 工具轉換
 */
for (i = 0; i < depth; i++) {
    printf("%08x ", call_stack[i]);
}
```

#### 2.4.3 追蹤斷言錯誤資訊

```C
void cm_backtrace_assert(uint32_t sp)
```

|參數                                    |描述|
|:-----                                  |:----|
|sp                                      |斷言環境時的堆疊指針|

> **注意** ：入參 SP 儘量在斷言函式內部獲取，而且儘可能靠近斷言函式開始的位置。當在斷言函式的子函式中（例如：在 RT-Thread 的斷言鉤子方法中）使用時，由於函式巢狀會存在暫存器入棧的操作，此時再獲取 SP 將發生變化，就需要人為調整（加減固定的偏差值）入參值，所以作為新手 **不建議在斷言的子函式** 中使用該函式。

#### 2.4.4 追蹤故障錯誤資訊

```C
void cm_backtrace_fault(uint32_t fault_handler_lr, uint32_t fault_handler_sp)
```

|參數                                    |描述|
|:-----                                  |:----|
|fault_handler_lr                        |故障處理函式環境下的 LR 暫存器值|
|fault_handler_sp                        |故障處理函式環境下的 SP 暫存器值|

該函式可以在故障處理函式（例如： `HardFault_Handler`）中呼叫。另外，庫本身提供了 `HardFault` 處理的彙編檔案（[點選檢視](https://github.com/armink/CmBacktrace/tree/master/cm_backtrace/fault_handler)，需根據自己編譯器進行選擇），會在故障時自動呼叫 `cm_backtrace_fault` 方法。所以移植時，最簡單的方式就是直接使用該彙編檔案。

### 2.5 常見問題

#### 2.5.1 編譯出錯，提示需要 C99 支援

[點選檢視教程：一步開啟 Keil/IAR/GCC 的 C99 支援](https://github.com/armink/CmBacktrace/blob/master/docs/zh/enable_c99_for_keil_iar_gcc.md)

#### 2.5.2 如何檢視到函式呼叫棧中函式的具體名稱及程式碼行號

[點選檢視教程：如何使用 addr2line 工具獲取函式呼叫棧詳細資訊](https://github.com/armink/CmBacktrace/blob/master/docs/zh/how_to_use_addr2line_for_call_stack.md)

#### 2.5.3 故障處理函式：HardFault_Handler 重複定義

在使用了本庫提供的 cmb_fault.s 彙編檔案時，因為該彙編檔案內部已經定義了 HardFault_Handler ，所以如果專案中還有其他地方定義了該函式，則會提示 HardFault_Handler 被重複定義的錯誤。此時有兩種解決方法：

- 1、註釋/刪除其他檔案中定義的 `HardFault_Handler` 函式，僅保留 cmb_fault.s 中的；
- 2、將 cmb_fault.s 移除工程，手動新增 `cm_backtrace_fault` 函式至現有的故障處理函式，但需要注意的是，務必 **保證該函式數入參的準備性** ，否則可能會導致故障診斷功能及堆疊列印功能無法正常執行。所以如果是新手，不推薦第二種解決方法。

#### 2.5.4 初始化時提示無法獲取主棧（main stack）資訊

在 `cmd_def.h` 中有定義預設的主棧配置，大致如下：

```c

#if defined(__CC_ARM)
    /* C stack block name, default is STACK */
    #ifndef CMB_CSTACK_BLOCK_NAME
    #define CMB_CSTACK_BLOCK_NAME          STACK
    #endif
    ...
#elif defined(__ICCARM__)
    /* C stack block name, default is 'CSTACK' */
    #ifndef CMB_CSTACK_BLOCK_NAME
    #define CMB_CSTACK_BLOCK_NAME          "CSTACK"
    #endif
    ...
#elif defined(__GNUC__)
    /* C stack block start address, defined on linker script file, default is _sstack */
    #ifndef CMB_CSTACK_BLOCK_START
    #define CMB_CSTACK_BLOCK_START         _sstack
    #endif
    /* C stack block end address, defined on linker script file, default is _estack */
    #ifndef CMB_CSTACK_BLOCK_END
    #define CMB_CSTACK_BLOCK_END           _estack
    #endif
    ...
#else
```

比如在 Keil-MDK 編譯器下會預設選擇 `STACK` 作為主棧 block 的名稱，但在一些特殊平臺下，專案的主棧 block 名稱可能不叫 `STACK`，導致 CmBacktrace 無法獲取到正確的主棧資訊，所以在初始化時會有如上的錯誤提示資訊。

解決這個問題一般有兩個思路

- 1、在 `cmb_cfg.h` 中重新定義主棧的資訊，此時 CmBacktrace 會優先使用 `cmb_cfg.h` 中的配置資訊；
- 2、修改專案配置，如果是 Keil-MDK ，則在啟動檔案的開頭位置，將主棧的名稱修改爲預設的 `STACK` ，其他編譯器一般很少出現該問題。

### 2.6 許可

採用 MIT 開源協議，細節請閱讀專案中的 LICENSE 檔案內容。
