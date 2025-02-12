﻿# RX対応 μT-Kernel3.0
　トロンフォーラムより公開されているμT-Kernel3.0をルネサスエレクトロニクス社のRX（RXv1～RXv3コア）用にポーティングしたソースコードです。

■ ターゲットボードは以下の10製品をサポートしています。

- GR-SAKURA（ルネサスエレクトロニクス社、RX63N(RXv1コア)：R5F563NBD搭載）
- AP-RX63N-0A　（アルファプロジェクト社、RX63N(RXv1コア)：R5F563NED搭載）
- AP-RX65N-0A　（アルファプロジェクト社、RX65N(RXv2コア)：R5F565N9B搭載）
- AP-RX72N-0A　（アルファプロジェクト社、RX72N(RXv3コア)：R5F572NND搭載）
- TB-RX231 （ルネサスエレクトロニクス社、RX231(RXv2コア)：R5F52318A搭載）
- TB-RX23W （ルネサスエレクトロニクス社、RX23W(RXv2コア)：R5F523W8A搭載）
- TB-RX65N （ルネサスエレクトロニクス社、RX65N(RXv2コア)：R5F565NED搭載）
- TB-RX660 （ルネサスエレクトロニクス社、RX660(RXv3コア)：R5F56609B搭載）
- TB-RX66N （ルネサスエレクトロニクス社、RX66N(RXv3コア)：R5F566NNH搭載）
- EK-RX72N （ルネサスエレクトロニクス社、RX72N(RXv3コア)：R5F572NNH搭載）

■ 開発環境はルネサスエレクトロニクス社のCS+、コンパイラはCC-RXです。

■ RXv1～RXv3コア、Little/Big Endianに対応しており、他の品種へのポーティングも容易です。

■ RXv3コアは倍精度浮動小数点、割込みのレジスタバンクに対応しています。

■ スタック見積もりツールにより、各種スタックサイズの算出を簡単に行えます。

■ パートナーOS対応デバッグプラグインによりカーネルオブジェクトの状態参照が可能です。

■ TCP/IPプロトコルスタック[M3S-T4-Tiny]を利用したイーサネット通信をサポートしました。

■ FatFsを利用したファイルシステムをサポートしました。

■ メモリデバイスとしてRAMディスクとSDカードとUSBメモリをサポートしました。

■ 現在、以下のデバイスドライバをサポートしています。

　- I2Cドライバ（内蔵RIIC利用、RX63N、RX231、RX23W、RX65N、RX660、RX66N、RX72N）

　- 簡易I2Cドライバ（内蔵SCI利用、RX63N、RX231、RX23W、RX65N、RX660、RX66N、RX72N）

　- シリアルドライバ（内蔵SCI利用、RX231、RX23W、RX65N、RX660、RX66N、RX72N）

　- データフラッシュドライバ（内蔵FLASH利用、RX65N）

■ RX72N Envision KitにおけるTouch PanelとTFT Displayのサンプルをリリースしました。

■ RX72N Envision KitのTFT Displayをキャラクタスクロールとして使用するサンプルをリリースしました。

■ RX72N Envision KitのQSPI接続Serial Flash Memoryを操作するドライバのサンプルをリリースしました。