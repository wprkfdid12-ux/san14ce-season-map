param([switch]$Uninstall,[string]$GameDir='')
$ErrorActionPreference='Stop'
Set-StrictMode -Version 2.0
$ScriptRoot=Split-Path -Parent $MyInvocation.MyCommand.Path
$PackageRoot=Split-Path -Parent $ScriptRoot
$ResultLog=$null
$Targets=@(
 @{Kind='TEX';Name='tile_bin14';File='FieldEditor4.rdb.bin14';FileLen=[long]99244096;Offset=[long]0;Len=[int]50652375;Orig='a1a508227736b7b4adbf378c7219fcea3cd5415e5c1805c9379a5579cae9df99';New='a9d311f7c9b5248e0e4a74790951827c977645adfe17b47e0c5b5c351f45c9eb';Prev='5b8e1d63e9c202020b8b270e968c0a92d44f88e0a7736f3726fb57ddbf4c0236|1f9ab4871542d52bbea3f28eff450e8c058d1912edc2d87c0ae24df1c51b2cae|6034400c2a2a590a7078582b825e85e377134ed6dffefeac7b3e6a23b1ca5382|385cebe8e728002a6232f631f1cf99fdbe739c264b1008ce678ebe34cf9b64ed|beecdbd8d5c60a2c1c9a74f7b39b3b3a8ee0537ff88ef2f9f1e62dc0cd58f2c5|32a133e962988765dced0f555af154a3486c7282e40e9db806cbed696cade918';Segs='O,0,88;O,88,6851858;P,0,582012;O,7361012,1961719;P,582012,443511;O,9805832,7897314;P,1025523,126936;O,17839686,474491;P,1152459,120955;O,18442554,1948106;P,1273414,42426;O,20435626,105334;P,1315840,40814;O,20583189,470476;P,1356654,21364;O,21076026,14278;P,1378018,21179;O,21111804,103185;P,1399197,23453;O,21238654,13826;P,1422650,25871;O,21278327,7310589;P,1448521,997842;O,29596556,2042286;P,2446363,994127;O,32633517,8053774;P,3440490,267189;O,40954589,496358;P,3707679,263427;O,41713760,2025095;P,3971106,79389;O,43818289,111567;P,4050495,77749;O,44007965,502127;P,4128244,31927;O,44542164,15916;P,4160171,31162;O,44589397,114311;P,4191333,26970;O,44730855,15120;P,4218303,23540;O,44769661,5882710;O,50652371,4';Optional=$false},
 @{Kind='TEX';Name='tile_bin14_0';File='FieldEditor4.rdb.bin14_0';FileLen=[long]135551516;Offset=[long]65536;Len=[int]50652375;Orig='544f2517d739df961ee8a1dbecd900093fc6330f169d79b011483fd771bf5275';New='fccf981d418a80a0106cc4c96ced551381f765b2e03e1857e3f437c0c4beb863';Prev='379fa827a2c719c605ec6f8ac510d47057814cdcb89265bfa83171e435a8b07e|d3df59ad9f89b98639ccf526f02a4a4e9a31be03296315228c28795bbcd2e225|22c354e6b8ad41e19af81ce2e0626627776ea025f3980950e0720724e9dc5411|b90f1701ed28018257eebed2eefb7b0da3504f80e9bcf6245a96c893d921d80a|7e060feb7f373592fe6ba3bd954e3a2134d3109d006c62725cacb8c883f862df|c4c205745bc168164238fda8a088d2f04fc1790d88a60a77831b620169501a10';Segs='O,0,88;O,88,6851858;P,0,582012;O,7361012,1961719;P,582012,443511;O,9805832,7897314;P,1025523,126936;O,17839686,474491;P,1152459,120955;O,18442554,1948106;P,1273414,42426;O,20435626,105334;P,1315840,40814;O,20583189,470476;P,1356654,21364;O,21076026,14278;P,1378018,21179;O,21111804,103185;P,1399197,23453;O,21238654,13826;P,1422650,25871;O,21278327,7310589;P,1448521,997842;O,29596556,2042286;P,2446363,994127;O,32633517,8053774;P,3440490,267189;O,40954589,496358;P,3707679,263427;O,41713760,2025095;P,3971106,79389;O,43818289,111567;P,4050495,77749;O,44007965,502127;P,4128244,31927;O,44542164,15916;P,4160171,31162;O,44589397,114311;P,4191333,26970;O,44730855,15120;P,4218303,23540;O,44769661,5882710;O,50652371,4';Optional=$false},
 @{Kind='TEX';Name='tile_bin';File='FieldEditor4.rdb.bin';FileLen=[long]134634012;Offset=[long]65536;Len=[int]50650533;Orig='c3783e2df9ba1fd238f616027c86f3cf5fdbd4c38b623c0c1b043b08169dd798';New='904bd7714acc1d19d85b1e823b06e4a9f8ecd7515f010a2f59271bdce885e106';Prev='2fb7ee84505c0ac1e8dff5563866c519995dd9259ff255fcfd0d62bb8657efd4|b065681d53a16a9fa5323ab9f5008fa15bb7c8f40e9cad0d1cdd55a34c439664|0c23e3452ff9ec68537b1b92769a344adf12e6273975adfcf170d3ee7d1ae5ea|13f06b2b26ce80fc4707288391e9989f8ad57981a2b89f6d84178f337df6304a|ceb13299035d9c73fe413de78bef2c7444fb129b9c2eaf435fa0a99e6fa12c80|1886741e9c748213c83b5371e08ecabbda864e3cbbf5ab25bcd66ceccc205412';Segs='O,0,88;O,88,6851856;P,4241843,581950;O,7361010,1961719;P,4823793,443511;O,9805830,7897489;P,5267304,126936;O,17839859,474492;P,5394240,120955;O,18442728,1948136;P,5515195,42426;O,20435825,105336;P,1315840,40814;O,20583390,470478;P,5557621,21364;O,21076229,14278;P,5578985,21179;O,21112007,103185;P,1399197,23453;O,21238857,13826;P,1422650,25871;O,21278530,7309619;P,1448521,997842;O,29595788,2042308;P,5600164,994126;O,32632770,8053120;P,6594290,267251;O,40953234,496376;P,6861541,263439;O,41712419,2025085;P,7124980,79384;O,43816932,111546;P,7204364,77745;O,44006564,502121;P,7282109,31931;O,44540758,15915;P,7314040,31163;O,44587992,114315;P,7345203,26970;O,44729454,15119;P,7372173,23542;O,44768259,5882270;O,50650529,4';Optional=$false},
 @{Kind='TEX';Name='tile_bin2';File='FieldEditor4.rdb.bin2';FileLen=[long]134634012;Offset=[long]65536;Len=[int]50650533;Orig='c3783e2df9ba1fd238f616027c86f3cf5fdbd4c38b623c0c1b043b08169dd798';New='904bd7714acc1d19d85b1e823b06e4a9f8ecd7515f010a2f59271bdce885e106';Prev='2fb7ee84505c0ac1e8dff5563866c519995dd9259ff255fcfd0d62bb8657efd4|b065681d53a16a9fa5323ab9f5008fa15bb7c8f40e9cad0d1cdd55a34c439664|0c23e3452ff9ec68537b1b92769a344adf12e6273975adfcf170d3ee7d1ae5ea|13f06b2b26ce80fc4707288391e9989f8ad57981a2b89f6d84178f337df6304a|ceb13299035d9c73fe413de78bef2c7444fb129b9c2eaf435fa0a99e6fa12c80|1886741e9c748213c83b5371e08ecabbda864e3cbbf5ab25bcd66ceccc205412';Segs='O,0,88;O,88,6851856;P,4241843,581950;O,7361010,1961719;P,4823793,443511;O,9805830,7897489;P,5267304,126936;O,17839859,474492;P,5394240,120955;O,18442728,1948136;P,5515195,42426;O,20435825,105336;P,1315840,40814;O,20583390,470478;P,5557621,21364;O,21076229,14278;P,5578985,21179;O,21112007,103185;P,1399197,23453;O,21238857,13826;P,1422650,25871;O,21278530,7309619;P,1448521,997842;O,29595788,2042308;P,5600164,994126;O,32632770,8053120;P,6594290,267251;O,40953234,496376;P,6861541,263439;O,41712419,2025085;P,7124980,79384;O,43816932,111546;P,7204364,77745;O,44006564,502121;P,7282109,31931;O,44540758,15915;P,7314040,31163;O,44587992,114315;P,7345203,26970;O,44729454,15119;P,7372173,23542;O,44768259,5882270;O,50650529,4';Optional=$true},
 @{Kind='TEX';Name='cliffA_bin14_0';File='FieldEditor4.rdb.bin14_0';FileLen=[long]135551516;Offset=[long]126812160;Len=[int]2906529;Orig='70f4ec1d2b28cc3a3e1ee2d4e057af2c8dffb9179932483abbc8fbd0e15b7df6';New='70f4ec1d2b28cc3a3e1ee2d4e057af2c8dffb9179932483abbc8fbd0e15b7df6';Prev='f322c5999e9d1a5ac642dba397c0457823125265d36e223a3bdc987b5723c6ba|96a8ca2f4c632099c0a6d1fac12b0a4b6b7e649291a1e236f3d0c1b40f93595d';Segs='O,0,2906529';Optional=$false},
 @{Kind='TEX';Name='cliffA_bin';File='FieldEditor4.rdb.bin';FileLen=[long]134634012;Offset=[long]125894656;Len=[int]2906529;Orig='e0f790436d3b66668aae87bc6f044d2caf15200612083252e1f28a9eae331d76';New='e0f790436d3b66668aae87bc6f044d2caf15200612083252e1f28a9eae331d76';Prev='6eae8ee3135e7c81fdbcffcd57fac8a5112dad58fda523e474b8af2f7f0537c6|c0184eb2f52fe80fb0a890e4aed8fa6b59d4dccc34b2c8582286c909267f99f0';Segs='O,0,2906529';Optional=$false},
 @{Kind='TEX';Name='cliffA_bin2';File='FieldEditor4.rdb.bin2';FileLen=[long]134634012;Offset=[long]125894656;Len=[int]2906529;Orig='e0f790436d3b66668aae87bc6f044d2caf15200612083252e1f28a9eae331d76';New='e0f790436d3b66668aae87bc6f044d2caf15200612083252e1f28a9eae331d76';Prev='6eae8ee3135e7c81fdbcffcd57fac8a5112dad58fda523e474b8af2f7f0537c6|c0184eb2f52fe80fb0a890e4aed8fa6b59d4dccc34b2c8582286c909267f99f0';Segs='O,0,2906529';Optional=$true},
 @{Kind='TEX';Name='cliffN_bin14_0';File='FieldEditor4.rdb.bin14_0';FileLen=[long]135551516;Offset=[long]129761280;Len=[int]3682608;Orig='b42010a861af0484465dd2450b49f12838d6cda4568a3c72535942f4c3f8b133';New='b42010a861af0484465dd2450b49f12838d6cda4568a3c72535942f4c3f8b133';Prev='5503d2b4235f964d329ac6b82b4bf470bcd5a5ffb79f5558c2547040255a2adf|d9d5514c25debe730daa0c4dbca424862d307c74510fcb3b47df1b793da72e00';Segs='O,0,3682608';Optional=$false},
 @{Kind='TEX';Name='cliffN_bin';File='FieldEditor4.rdb.bin';FileLen=[long]134634012;Offset=[long]128843776;Len=[int]3682608;Orig='d14d24b11bce1e17bc9c0f7102d1a1befdeec2de687c0e3ed19af70bfcbc1115';New='d14d24b11bce1e17bc9c0f7102d1a1befdeec2de687c0e3ed19af70bfcbc1115';Prev='bd77e77bbb08d523289fb35bfdc25786e1bf893dd4c5d38c1de9564b24794f19|e66384a2e472251da3cb14466d945e2f639c72116d8e798345b68025e3222c0d';Segs='O,0,3682608';Optional=$false},
 @{Kind='TEX';Name='cliffN_bin2';File='FieldEditor4.rdb.bin2';FileLen=[long]134634012;Offset=[long]128843776;Len=[int]3682608;Orig='d14d24b11bce1e17bc9c0f7102d1a1befdeec2de687c0e3ed19af70bfcbc1115';New='d14d24b11bce1e17bc9c0f7102d1a1befdeec2de687c0e3ed19af70bfcbc1115';Prev='bd77e77bbb08d523289fb35bfdc25786e1bf893dd4c5d38c1de9564b24794f19|e66384a2e472251da3cb14466d945e2f639c72116d8e798345b68025e3222c0d';Segs='O,0,3682608';Optional=$true},
 @{Kind='TEX';Name='cliffM_bin14_0';File='FieldEditor4.rdb.bin14_0';FileLen=[long]135551516;Offset=[long]133496832;Len=[int]2034554;Orig='672179a4f4a692abecb7e6ac22a262e4a2feb7a95e284685244ec63cf8c32a36';New='672179a4f4a692abecb7e6ac22a262e4a2feb7a95e284685244ec63cf8c32a36';Prev='d148fdfd91f277ee0d363a7a882270a511c818c357615f1330869f0106a0f3c5|8dae0024acc687258621eaa5a994ad6e8fd3f50fff83ad7db8233ce9f0d0c3ca';Segs='O,0,2034554';Optional=$false},
 @{Kind='TEX';Name='cliffM_bin';File='FieldEditor4.rdb.bin';FileLen=[long]134634012;Offset=[long]132579328;Len=[int]2034554;Orig='cdd755548c11920d823a9918b81c9453c4ea4a1615de1d5e54935aa484fb3096';New='cdd755548c11920d823a9918b81c9453c4ea4a1615de1d5e54935aa484fb3096';Prev='8376a4e1eef60b659c4323d6c59c93a20b87f88a2c3c12438d9f3b6e213f3d4f|39a9fe537a81739773f0c0184d73648ce8298fb8633928586ce94704117b7412';Segs='O,0,2034554';Optional=$false},
 @{Kind='TEX';Name='cliffM_bin2';File='FieldEditor4.rdb.bin2';FileLen=[long]134634012;Offset=[long]132579328;Len=[int]2034554;Orig='cdd755548c11920d823a9918b81c9453c4ea4a1615de1d5e54935aa484fb3096';New='cdd755548c11920d823a9918b81c9453c4ea4a1615de1d5e54935aa484fb3096';Prev='8376a4e1eef60b659c4323d6c59c93a20b87f88a2c3c12438d9f3b6e213f3d4f|39a9fe537a81739773f0c0184d73648ce8298fb8633928586ce94704117b7412';Segs='O,0,2034554';Optional=$true},
 @{Kind='TEX';Name='mhk_948';File='FieldEditor4.rdb.bin14_0';FileLen=[long]135551516;Offset=[long]88801280;Len=[int]11927651;Orig='1e7d14ad4f13ec210b6f49bdd6779e476ab4ba27f9867797c470b6673408dbfc';New='40f3da00fcc3f5d1cc70f71868aa98154f9ef0335f32ea6041dfb6ea009ebd0a';Prev='f0038c94587a4c56e3d6b75dae5b01bcd3c31e6e88734eed8ef83082529609be|0aa29bc997829ee20a46df78851a2a2892c4345c63c85e321563aff9bc7c876d|c92832408861b2425e9f7ec78980d6e992770e8d8ade123c3e9cb2a47f5c0e99|3dd884d27d7258eae72575f169e7a9835fa127754885294a7e4cd83ce400a09b|bddb0bd995998db481f951f8248beffb1abb72869c1988fd3629921a99695e1e|8021a5b25a14d791b6eb81c6b44857161ec8bf5b9b3f1bdc433238bcec69f1d5|86d8f89d0b590af0604e36f0ff5a7399a4090e3e633ad631e334949dd8d16da0|00f0e92a450ef06e390b51e4b3af156e260fef9f49112522fca14629dee21ba6|5200608ca31153e23750c81b10aff04c9aac106f62e63d0eb6206f4a52f0aa5a|64e3f4571d4000d7b62f08a4ce36c37bc8f62b1782015fa177b23904d73d55b0|3303dce4dfa4bf66e2ffdafcb3b8d02f3cdb9a3715c52ea253a7c6f61bcafd91|21d005c2c3705870ba7a8d1e465d37ae3691f7a526b31da4b207f83c4988e992|931a02efb1fff11423367113b695f66ba829364f42a939281110d97fe0ef8569|c40f36ab9885a5deb80279c724eb09cf6e611cd8873a40a393fa54f46310e10d|0952ca956ff5ee310e4c71938712830302bca9cac3f47183a678d3385a986494|6d0e5ce2b5ef4e8612c90ef0ad80805092146f9ee2fa7b8f3b9056c532a2b921';Segs='O,0,56;P,7395715,11927573;O,11927629,18;O,11927647,4';Optional=$false},
 @{Kind='TEX';Name='mhk_613';File='FieldEditor4.rdb.bin';FileLen=[long]134634012;Offset=[long]88866816;Len=[int]12127543;Orig='abd0fb6adbec6d22e4e0b007eae39f64267568caa6e7727285f45d7e16424082';New='45a70c717787f914eeb8b8a9d6610f6e34c61dbba89dac7632c19b23406f37f4';Prev='5e1a97555258bf566e7b59bafdcc879b5eac08d5e3b98557ed4adf82a87183c4|8dd5c964eec8ae69d78fc0cea4dcfbdb0b11fd48c84c3cd46fcf0dd43d24f96b|f48ea918b810f43629a6f477b3c393376a665622ce1a21dbccb815f947436c72|de1caafd7a9dc70f22266d881f30807817c3654640dd566d1b21984957b3e00e|d9a6b79cd25057af9461697ec2b30e0d070af69a3fe8eb39157cc2ac6e76c35d|03662be7f3a586b99bcf199de90585256a5feba6d4b007db1edf6590a9a26cf1|c3a37da6407c3e5713b7d7d607cacad61df3f4526f71c26d562276bbfebab262|59447558eb96e89938b770c01c29332dda979d9d0ff10a03021e33a9cecc8265|e6f5b1f8f7f4e25db9ff5c97f2f951f6bf7b8b6dc83e82aa6b97c3c3fbd2c392|4a33256cf7c97e29d7edd6ec01a7634a32d620d6965afb2f9b1674e7bf90bc8d|e3a4396a2f45a7a830a44de7c9d21dc771c0e1a283e39ece331859c7539f0a98|74c90de43defc1f82f99d702824f25fe202c0dcf3e043c90d7a744273e4d5e83|b2a7bfae9124268059a88ac6061cfa621e1ff417e9b2fe092b413685409c49ac|09dcb766bed5eb8b052287a369bee83e55fc301fdd6c755331415be40438e29a|8816c410d9882c01dcb464229a32a73d2f33c9283574f5bb064442d233dbbd35|2336776150d94748a0ae249ab390dd2ca6299734db36f6f833034a616c574b6a';Segs='O,0,56;P,19323288,12127465;O,12127521,18;O,12127539,4';Optional=$false},
 @{Kind='TEX';Name='mhk_613b2';File='FieldEditor4.rdb.bin2';FileLen=[long]134634012;Offset=[long]88866816;Len=[int]12127543;Orig='abd0fb6adbec6d22e4e0b007eae39f64267568caa6e7727285f45d7e16424082';New='45a70c717787f914eeb8b8a9d6610f6e34c61dbba89dac7632c19b23406f37f4';Prev='5e1a97555258bf566e7b59bafdcc879b5eac08d5e3b98557ed4adf82a87183c4|8dd5c964eec8ae69d78fc0cea4dcfbdb0b11fd48c84c3cd46fcf0dd43d24f96b|f48ea918b810f43629a6f477b3c393376a665622ce1a21dbccb815f947436c72|de1caafd7a9dc70f22266d881f30807817c3654640dd566d1b21984957b3e00e|d9a6b79cd25057af9461697ec2b30e0d070af69a3fe8eb39157cc2ac6e76c35d|03662be7f3a586b99bcf199de90585256a5feba6d4b007db1edf6590a9a26cf1|c3a37da6407c3e5713b7d7d607cacad61df3f4526f71c26d562276bbfebab262|59447558eb96e89938b770c01c29332dda979d9d0ff10a03021e33a9cecc8265|e6f5b1f8f7f4e25db9ff5c97f2f951f6bf7b8b6dc83e82aa6b97c3c3fbd2c392|4a33256cf7c97e29d7edd6ec01a7634a32d620d6965afb2f9b1674e7bf90bc8d|e3a4396a2f45a7a830a44de7c9d21dc771c0e1a283e39ece331859c7539f0a98|74c90de43defc1f82f99d702824f25fe202c0dcf3e043c90d7a744273e4d5e83|b2a7bfae9124268059a88ac6061cfa621e1ff417e9b2fe092b413685409c49ac|09dcb766bed5eb8b052287a369bee83e55fc301fdd6c755331415be40438e29a|8816c410d9882c01dcb464229a32a73d2f33c9283574f5bb064442d233dbbd35|2336776150d94748a0ae249ab390dd2ca6299734db36f6f833034a616c574b6a';Segs='O,0,56;P,19323288,12127465;O,12127521,18;O,12127539,4';Optional=$true},
 @{Kind='TEX';Name='tex_bin14_0';File='FieldEditor4.rdb.bin14_0';FileLen=[long]135551516;Offset=[long]57868288;Len=[int]30911541;Orig='33af22b3041224a9a07035721ee43336e6b86c7e729f9b559a88e31d16c40262';New='30b69663c82ff937276d535fcf118adc34eccb8a88212c3b2ec88ffbdf82d265';Prev='c344eea79f211815e66aadea79f7f05603e3835dcbca94c30d977ff6ecf54628|a0e836cb5908a619fd3eafa19684201477c325fab02be3debc727f5c5940d37b|e06bccb7a369de3728dc431b8983c719f7967bbf410f16de7fb106d5139f8931|bf0f29a28894564c026f45ee115454173d22407965a5bb29fe5890dc62e82695|82544b8afa7cab510bd6b869b0d83334b8e466dee4b9187c873ff6f448a8946b|017bc7ea6273d95271f947ef9bedaaebd1e27be43befaee2fc3e4f574dc2ef9c|4f6fa33655e3da5b5a20302e7b2a9612f8b502ab331f6b296828d2e8c85c0480|0d3bd3d440fafe4ee11dc704d5d2b7ffa57f865cb7a33627cfc18d040b8c27b5|4e9b886dcdb0569f3a6cd975f3b09ed257b46fadc0760546f79fe7db4acf8ec9|d7311a06ac198870dfba901f03614825ed672f77bed3426ecd90b16ddb29ec0f|4fc2b955b5fba49c1eec09956b61e10d926ffb61a39ba3338aae01c57beb5a14|7728181de2cf246b8d486bd475564d4897b597eb58646c88ba202a226bfd02a6|c335b00aab3cc090c50f4c9f3ecd5f0d5298661743ae35273afe635eb8c3494f|634d7af9de8bde1545c69e6de8593b0d370ffd2cee6dc08bc6b28c283a005672';Segs='O,0,88;P,31450753,12670908;O,12670996,18240541;O,30911537,4';Optional=$false},
 @{Kind='TEX';Name='tex_bin14';File='FieldEditor4.rdb.bin14';FileLen=[long]99244096;Offset=[long]57802752;Len=[int]30968596;Orig='c736987d0f040758661fbc08415afbd91d49f5320b3c4130f39c53bbe3bea10c';New='1465e62fd11fd3216b6d806040d43d8807d32c9998d123e4d3d6319be03ef457';Prev='d90b4556790e5d0904d86735be004f97b86e6910f8725571bfae8ce7f53c8164|491749082b5e37e7bc8cfdca85f23a585d8086d1590572db7da06b0fc776ae00|1e2bd5ccc445490d0eb2abdf35f6d8ed3eb981ea49a1e5a62a34bcefabc48bcf|5803b5dc688872f51bc9ac35518e586d935f722d48bd7ef58719c094e49b4f8d|312638c1fae890f3e53d899853ed7aff1f63cc58a9fe08670c489c21228909ba|9f0fe7743767a66810820f7c54b803cd1ea0e9fa93b8693f5e7b5d9a6e1154b0|91312939a53cd1e8cabdf4f871a5f2827cf4b840e812b9c4eb3bae68702ba07e|4a131a2f7716d7ed4a02128cb20b88614fd7b9ee99771c25862d9274cf0c1424|cce58b424da3ac0ae03d27e5486d864ed1f14b6302b3d998495c461c9e597254|cf7797871fc5be06737c851484a26d1a7fe0cb64f73ca7f0bdb6bea5d63c250d|3e978bc4c27ef63e1eeb15e268c399e00001aee99c125db160546c7f71a2d752|fcf1109c4c0f6936f7c3860e8d8873848aa770d30c6e6e3c82a3845ea55f9916|0b1e7e2240e4a370232d2a5208806ad9476350b6506a50091bc4c735d5ba0fe4|0b3fb64283482e40911b0651d3d8134d159e0b1712b8d681456cc4245d40acc5';Segs='O,0,88;P,44121661,12884034;O,12884122,18084470;O,30968592,4';Optional=$false},
 @{Kind='TEX';Name='tex_bin';File='FieldEditor4.rdb.bin';FileLen=[long]134634012;Offset=[long]57868288;Len=[int]30965439;Orig='2c3d4b193b6342697db167c9fa1dced9347ccb2d1d2d68ec7f5a187ae348a39e';New='d53be5fb9c533e457bb6e5290476a8dd38c30390cd24199e74e63081344e1b99';Prev='40f605d5cc794aa32b71b16ec102d4e76645d94d044c5852b5f27128fa822fc0|1de681ad5def9d8a2aa6ad338d6f08dcd410005f981d5ca07de1833c4a29b514|aac7ceb5032e815ef582385d52645065480215ac03bfc46a39a25ccc15041f88|abb8d563653aea1dbf3858b5972ac21ea85919f71120b5e3dc3b0a6443979fef|b53605fde5d556b426e4be2bc6a88161d790a737fb53c8f4616b35607d97534f|e7bb8a68fcf2d1c44790eb958e55940b74e5f8475e9ed6a5e47b8c0f1ba306ca|36c7024d642a5899550e8c400a7cd779506304a8ce2cbbccaf533799be6817dd|b2cceb34b4fbacb1a5bf3e7c29183b5ecf9ca13a07ec38d2e1391295ef7ffb4b|a9623088baf5086f6ee76d61e027e2a07d3081e0aaff4ce4e799a309ee9c3fd4|7a0d63a5165b4e8fe7c12bd8f9ac03587f9426e6a4ae50cce173e313b92a94a2|5a9ce914164b2acebd6aaed568f958fc011b43992ffcf3fb13b1f209eb75f87e|32a02acc2f740f4c8ee02690f644e87066d7ebb6443d6b554dbebf9027237e35|06341c4c66f87d9c50bac1f61efdd3839849614d746bad5ca17c7bb2ee294ea9|7c3edc762261401b0cc0a27b963d200e3dd34c9bf347236bf3e4abdb0a9c8c04';Segs='O,0,88;P,44121661,12884034;O,12884122,18081313;O,30965435,4';Optional=$false},
 @{Kind='TEX';Name='tex_bin2';File='FieldEditor4.rdb.bin2';FileLen=[long]134634012;Offset=[long]57868288;Len=[int]30965439;Orig='2c3d4b193b6342697db167c9fa1dced9347ccb2d1d2d68ec7f5a187ae348a39e';New='d53be5fb9c533e457bb6e5290476a8dd38c30390cd24199e74e63081344e1b99';Prev='40f605d5cc794aa32b71b16ec102d4e76645d94d044c5852b5f27128fa822fc0|1de681ad5def9d8a2aa6ad338d6f08dcd410005f981d5ca07de1833c4a29b514|aac7ceb5032e815ef582385d52645065480215ac03bfc46a39a25ccc15041f88|abb8d563653aea1dbf3858b5972ac21ea85919f71120b5e3dc3b0a6443979fef|b53605fde5d556b426e4be2bc6a88161d790a737fb53c8f4616b35607d97534f|e7bb8a68fcf2d1c44790eb958e55940b74e5f8475e9ed6a5e47b8c0f1ba306ca|36c7024d642a5899550e8c400a7cd779506304a8ce2cbbccaf533799be6817dd|b2cceb34b4fbacb1a5bf3e7c29183b5ecf9ca13a07ec38d2e1391295ef7ffb4b|a9623088baf5086f6ee76d61e027e2a07d3081e0aaff4ce4e799a309ee9c3fd4|7a0d63a5165b4e8fe7c12bd8f9ac03587f9426e6a4ae50cce173e313b92a94a2|5a9ce914164b2acebd6aaed568f958fc011b43992ffcf3fb13b1f209eb75f87e|32a02acc2f740f4c8ee02690f644e87066d7ebb6443d6b554dbebf9027237e35|06341c4c66f87d9c50bac1f61efdd3839849614d746bad5ca17c7bb2ee294ea9|7c3edc762261401b0cc0a27b963d200e3dd34c9bf347236bf3e4abdb0a9c8c04';Segs='O,0,88;P,44121661,12884034;O,12884122,18081313;O,30965435,4';Optional=$true}
)
$PaySha=@{}
$PaySha['season26.part1']='cf1181c364e9cdea74c048fb6f971e751318e8fe419ff3e5afe4932d96a3e4c4'
$PaySha['season26.part2']='f6fac4eb7ebad8625379ecddf01bdbf315b8290a9546107f73eea743c2c06173'
$PaySha['season26.part3']='e75bfc66bc3f1c90610d30046e448dfca0e31eb4f33d46b5c6453f7c4c01f18a'
$script:LogBuf=New-Object System.Collections.Generic.List[string]
function Log([string]$s){$line=('['+(Get-Date -Format 'HH:mm:ss')+'] '+$s);Write-Host $line;if($script:ResultLog){try{Add-Content -LiteralPath $script:ResultLog -Encoding UTF8 -Value $line}catch{}}else{$script:LogBuf.Add($line)}}
function Set-LogGame([string]$g){try{$script:ResultLog=Join-Path $g '_SAN14_SeasonMap_terrain_log.txt';Set-Content -LiteralPath $script:ResultLog -Encoding UTF8 -Value ('==== '+(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')+' ====');foreach($l in $script:LogBuf){Add-Content -LiteralPath $script:ResultLog -Encoding UTF8 -Value $l};$script:LogBuf.Clear()}catch{}}
function Find-GameRoot {
  if($GameDir -and (Test-Path -LiteralPath (Join-Path $GameDir 'SAN14CE_KO.exe'))){return $GameDir.TrimEnd('\')}   # 2026-11-11: 설치기 창이 고른 게임 폴더
  $candidates=@();$here=$PackageRoot
  for($k=0;$k -lt 4 -and $here;$k++){if(Test-Path (Join-Path $here 'SAN14CE_KO.exe')){$candidates += $here};$next=Split-Path -Parent $here;if($next -eq $here){break};$here=$next}
  $roots=@();foreach($rp in @('HKCU:\Software\Valve\Steam','HKLM:\SOFTWARE\WOW6432Node\Valve\Steam','HKLM:\SOFTWARE\Valve\Steam')){try{$v=Get-ItemProperty -Path $rp -ErrorAction Stop;if($v.PSObject.Properties['SteamPath'] -and $v.SteamPath){$roots += [string]$v.SteamPath};if($v.PSObject.Properties['InstallPath'] -and $v.InstallPath){$roots += [string]$v.InstallPath}}catch{}}
  foreach($sr in ($roots|Select-Object -Unique)){if(-not(Test-Path $sr)){continue};$libs=@($sr);$vdf=Join-Path $sr 'steamapps\libraryfolders.vdf';if(Test-Path $vdf){foreach($line in Get-Content $vdf -ErrorAction SilentlyContinue){if($line -match '"path"\s+"([^"]+)"'){$p=$Matches[1].Replace('\\','\');if(Test-Path $p){$libs += $p}}}};foreach($lr in ($libs|Select-Object -Unique)){$common=Join-Path $lr 'steamapps\common';if(Test-Path $common){foreach($d in Get-ChildItem $common -Directory -ErrorAction SilentlyContinue){if(Test-Path (Join-Path $d.FullName 'SAN14CE_KO.exe')){$candidates += $d.FullName}}}}}
  $hit=$candidates|Select-Object -Unique|Select-Object -First 1;if($hit){return $hit}
  Add-Type -AssemblyName System.Windows.Forms;$dlg=New-Object System.Windows.Forms.OpenFileDialog;$dlg.Filter='SAN14CE_KO.exe|SAN14CE_KO.exe';if($dlg.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK){throw '게임 폴더 선택이 취소되었습니다.'};return (Split-Path -Parent $dlg.FileName)
}
Add-Type -TypeDefinition @"
using System;
using System.IO;
using System.Security.Cryptography;
public static class MS4 {
 static string Hex(byte[] b){var s=new char[b.Length*2];const string h="0123456789abcdef";for(int i=0;i<b.Length;i++){s[i*2]=h[b[i]>>4];s[i*2+1]=h[b[i]&15];}return new string(s);}
 public static byte[] Read(string path,long off,int len){using(var f=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.Read)){if(off+len>f.Length)throw new InvalidDataException("file too short");f.Position=off;byte[] b=new byte[len];int p=0;while(p<len){int r=f.Read(b,p,len-p);if(r<=0)throw new EndOfStreamException();p+=r;}return b;}}
 public static string Sha(byte[] b){using(var s=SHA256.Create()){return Hex(s.ComputeHash(b));}}
 public static void Write(string path,long off,byte[] b){using(var f=new FileStream(path,FileMode.Open,FileAccess.ReadWrite,FileShare.None)){f.Position=off;f.Write(b,0,b.Length);f.Flush(true);}}
 public static void Copy(byte[] src,int so,byte[] dst,int doff,int n){Buffer.BlockCopy(src,so,dst,doff,n);}
}
"@
$script:MEOK=$true
try{
Add-Type -TypeDefinition @"
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Security.Cryptography;
// v26k47: put the season mountain heights into a copy of the height data that another mod (the map-expand mod) appended
// to the same game file and pointed the game at. Only cells where that copy still holds the original game value are changed,
// the copy keeps its exact byte size (re-compressed 16 KB pieces, padded with empty deflate blocks, no piece above 16000 bytes).
public static class SeasonME {
 public static List<string> Log = new List<string>();
 public static string LastProbes = ""; public static int LastApplied, LastAlready, LastTheirs;   // v26k48: a few cells of the copy and their season / original values (the installer checks them without inflating it all)
 const int CAP = 16000;
 static uint U32(byte[] b, int o) { return (uint)(b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24)); }
 static void P32(byte[] b, int o, uint v) { b[o] = (byte)v; b[o + 1] = (byte)(v >> 8); b[o + 2] = (byte)(v >> 16); b[o + 3] = (byte)(v >> 24); }
 public static string Sha(byte[] b) { using (var s = SHA256.Create()) { byte[] h = s.ComputeHash(b); var c = new char[h.Length * 2]; const string x = "0123456789abcdef"; for (int i = 0; i < h.Length; i++) { c[i * 2] = x[h[i] >> 4]; c[i * 2 + 1] = x[h[i] & 15]; } return new string(c); } }
 public static string AssetId(byte[] cont) { if (cont.Length < 48 || cont[0] != 0x49 || cont[1] != 0x44 || cont[2] != 0x52 || cont[3] != 0x4b) return ""; return BitConverter.ToString(cont, 36, 8); }

 // ---- container: IDRK header, then [u32 length][zlib piece]... ended by a zero length ----
 sealed class Cont {
  public byte[] Bytes; public int Hdr, End; public List<int> Pos = new List<int>(); public List<int> Len = new List<int>(); public int CH;
  public Dictionary<int, byte[]> Raw = new Dictionary<int, byte[]>();
  public Cont(byte[] b) {
   Bytes = b; Hdr = (int)(U32(b, 8) - U32(b, 16)); int p = Hdr;
   while (true) { int L = (int)U32(b, p); if (L == 0) break; Pos.Add(p + 4); Len.Add(L); p += 4 + L; if (p > b.Length - 4) throw new InvalidDataException("자료 조각이 끝을 넘음"); }
   End = p; CH = Piece(0).Length;
  }
  public byte[] Piece(int k) { byte[] r; if (Raw.TryGetValue(k, out r)) return r; r = Inflate(Bytes, Pos[k], Len[k]); Raw[k] = r; return r; }
  public long RawLen { get { return U32(Bytes, 24); } }
  // read n bytes of the raw (uncompressed) data at raw offset o
  public byte[] ReadRaw(long o, int n) { var outb = new byte[n]; int got = 0; while (got < n) { long q = o + got; int k = (int)(q / CH); int w = (int)(q % CH); byte[] pc = Piece(k); int c = Math.Min(n - got, pc.Length - w); if (c <= 0) throw new InvalidDataException("자료 끝을 넘어 읽음"); Buffer.BlockCopy(pc, w, outb, got, c); got += c; } return outb; }
 }
 static byte[] Inflate(byte[] b, int off, int len) {
  using (var ms = new MemoryStream(b, off + 2, len - 6)) using (var ds = new DeflateStream(ms, CompressionMode.Decompress)) using (var o = new MemoryStream()) { ds.CopyTo(o); return o.ToArray(); }
 }
 static byte[] RawDeflate(byte[] d, int off, int n) {
  using (var o = new MemoryStream()) { using (var ds = new DeflateStream(o, CompressionLevel.Optimal, true)) { ds.Write(d, off, n); } return o.ToArray(); }
 }
 static uint Adler(byte[] d) { uint a = 1, b = 0; for (int i = 0; i < d.Length; i++) { a = (a + d[i]) % 65521; b = (b + a) % 65521; } return (b << 16) | a; }
 // zlib piece of data with exactly 'size' bytes (null when impossible): [78 9C][empty stored blocks][stored block of the first m bytes][deflate of the rest][adler32]
 static byte[] Exact(byte[] data, int size) {
  for (int m = 0; m <= 80 && m < data.Length; m++) {
   byte[] df = RawDeflate(data, m, data.Length - m);
   int fixedLen = 2 + (m > 0 ? 5 + m : 0) + df.Length + 4;
   int rest = size - fixedLen;
   if (rest < 0) { if (m == 0) return null; continue; }
   if (rest % 5 != 0) continue;
   var o = new byte[size]; int p = 0; o[p++] = 0x78; o[p++] = 0x9C;
   for (int e = 0; e < rest / 5; e++) { o[p++] = 0; o[p++] = 0; o[p++] = 0; o[p++] = 0xFF; o[p++] = 0xFF; }
   if (m > 0) { o[p++] = 0; o[p++] = (byte)m; o[p++] = (byte)(m >> 8); o[p++] = (byte)~m; o[p++] = (byte)((~m) >> 8); Buffer.BlockCopy(data, 0, o, p, m); p += m; }
   Buffer.BlockCopy(df, 0, o, p, df.Length); p += df.Length;
   uint ad = Adler(data); o[p++] = (byte)(ad >> 24); o[p++] = (byte)(ad >> 16); o[p++] = (byte)(ad >> 8); o[p++] = (byte)ad;
   return o;
  }
  return null;
 }
 static byte[] Zlib(byte[] data) { byte[] df = RawDeflate(data, 0, data.Length); var o = new byte[df.Length + 6]; o[0] = 0x78; o[1] = 0x9C; Buffer.BlockCopy(df, 0, o, 2, df.Length); uint ad = Adler(data); int p = o.Length - 4; o[p] = (byte)(ad >> 24); o[p + 1] = (byte)(ad >> 16); o[p + 2] = (byte)(ad >> 8); o[p + 3] = (byte)ad; return o; }

 // where the height grid starts in the raw data and its width: kind 0 = MHK heights (u32, 32-byte head), 1 = G1T texture 0 (float, 28-byte head)
 static void Grid(Cont c, int kind, out long start, out int width) {
  if (kind == 0) { start = 32; width = (int)Math.Round(Math.Sqrt((c.RawLen - 32) / 4.0)); }
  else {
   byte[] h = c.ReadRaw(0, 64); uint to = U32(h, 0xc); byte[] t = c.ReadRaw(to, 8);
   long o0 = U32(t, 0) + to, o1 = U32(t, 4) + to; start = o0 + 28; width = (int)Math.Round(Math.Sqrt((o1 - o0 - 28) / 4.0));
  }
  if ((long)width * width * 4 + start > c.RawLen) throw new InvalidDataException("높이 판이 자료보다 큼");
 }
 static uint[] ReadGrid(Cont c, long start, int w) { byte[] b = c.ReadRaw(start, w * w * 4); var g = new uint[w * w]; Buffer.BlockCopy(b, 0, g, 0, b.Length); return g; }
 static uint Round64(uint bits) { float f = BitConverter.ToSingle(BitConverter.GetBytes(bits), 0); float r = (float)(Math.Round(f * 64.0) / 64.0); return BitConverter.ToUInt32(BitConverter.GetBytes(r), 0); }

 // mode 1 = install (orig -> season), 2 = check only, 3 = take the season back out (season -> orig: uninstall when the other mod's copy got
 // the season cells some other way, with no backup of ours). Returns the new container bytes (same length) or null when nothing to do / not possible.
 public static byte[] Merge(byte[] meCont, byte[] origCont, byte[] newCont, int kind, int mode, out string report) {
  report = ""; LastProbes = ""; LastApplied = 0; LastAlready = 0; LastTheirs = 0;
  var me = new Cont(meCont); var co = new Cont(origCont); var cn = new Cont(newCont);
  long so, sn, sm; int wo, wn, wm;
  Grid(co, kind, out so, out wo); Grid(cn, kind, out sn, out wn); Grid(me, kind, out sm, out wm);
  if (wo != wn) throw new InvalidDataException("순정·계절 높이 판 크기가 다름");
  uint[] go = ReadGrid(co, so, wo), gn = ReadGrid(cn, sn, wn);
  uint[] gr = (uint[])gn.Clone();   // v26k48: the season values as the season file has them (before rounding)
  if (kind == 1) for (int i = 0; i < gn.Length; i++) if (gn[i] != go[i]) gn[i] = Round64(gn[i]);
  // find where the original map sits inside the bigger grid: look for a 64-value run of the original
  int dr = -1, dc = -1;
  // probes: 64-value runs the season never changed (so they look the same before and after this merge) and that are not flat
  var probes = new List<int[]>();
  for (int r = wo / 7; r < wo - 1 && probes.Count < 8; r += 97) for (int c = wo / 9; c + 64 < wo && probes.Count < 8; c += 89) {
   int b0 = r * wo + c; bool keep = true, flat = true;
   for (int j = 0; j < 64; j++) { if (go[b0 + j] != gn[b0 + j]) { keep = false; break; } if (go[b0 + j] != go[b0]) flat = false; }
   if (keep && !flat) probes.Add(new int[] { r, c });
  }
  uint[] gm = ReadGrid(me, sm, wm);
  foreach (int[] pr in probes) {
   int r0 = pr[0], c0 = pr[1]; int baseI = r0 * wo + c0;
   for (int i = 0; i + 64 <= gm.Length && dr < 0; i++) {
    if (gm[i] != go[baseI]) continue; bool ok = true;
    for (int j = 1; j < 64; j++) if (gm[i + j] != go[baseI + j]) { ok = false; break; }
    if (ok) { int rr = i / wm - r0, cc = i % wm - c0; if (rr >= 0 && cc >= 0 && rr + wo <= wm && cc + wo <= wm) { dr = rr; dc = cc; } }
   }
   if (dr >= 0) break;
  }
  if (dr < 0) { report = "큰 지도 안에서 순정 땅 자리를 못 찾아 그대로 둠"; return null; }
  int same = 0, tot = 0; for (int r = 0; r < wo; r += 37) for (int cc2 = 0; cc2 < wo; cc2 += 41) { tot++; if (gm[(r + dr) * wm + cc2 + dc] == go[r * wo + cc2]) same++; }
  if (same * 3 < tot) { report = "큰 지도가 순정 땅과 " + same + "/" + tot + " 곳만 같아 그대로 둠"; return null; }
  int applied = 0, already = 0, theirs = 0, nseas = 0; var touched = new SortedSet<int>();
  var pb = new List<long>(); var ps = new List<uint>(); var po = new List<uint>();   // every 4096th cell that ends as season: probe candidates
  for (int r = 0; r < wo; r++) for (int c = 0; c < wo; c++) {
   int i = r * wo + c; if (go[i] == gn[i]) continue;
   int j = (r + dr) * wm + c + dc; uint v = gm[j]; uint tv;
   long bp = sm + 4L * j;
   if (mode == 3) {
    if (v == go[i]) { already++; continue; }
    if (v != gn[i] && v != gr[i]) { theirs++; continue; }
    tv = go[i];
   } else {
    if (v != gn[i] && v != go[i]) { theirs++; continue; }
    if ((nseas++ & 4095) == 0 && (bp % me.CH) + 4 <= me.CH) { pb.Add(bp); ps.Add(gn[i]); po.Add(go[i]); }
    if (v == gn[i]) { already++; continue; }
    tv = gn[i];
   }
   applied++;
   int k = (int)(bp / me.CH); int w = (int)(bp % me.CH);
   if (mode == 1 || mode == 3) {
    byte[] pc = me.Piece(k);
    if (w + 4 <= pc.Length) { P32(pc, w, tv); touched.Add(k); }
    else { // value split over two pieces
     byte[] nb = BitConverter.GetBytes(tv); for (int q = 0; q < 4; q++) { long bq = bp + q; int kq = (int)(bq / me.CH), wq = (int)(bq % me.CH); me.Piece(kq)[wq] = nb[q]; touched.Add(kq); }
    }
   }
  }
  if (pb.Count > 0) {   // 24 of them, evenly spread: "P<piece size>:<raw offset>=<season hex>/<original hex>,..."
   var sbp = new System.Text.StringBuilder(); sbp.Append("P").Append(me.CH).Append(":"); int want = Math.Min(24, pb.Count);
   for (int t = 0; t < want; t++) { int q = (int)((long)t * pb.Count / want); if (t > 0) sbp.Append(","); sbp.Append(pb[q]).Append("=").Append(ps[q].ToString("x8")).Append("/").Append(po[q].ToString("x8")); }
   LastProbes = sbp.ToString();
  }
  LastApplied = applied; LastAlready = already; LastTheirs = theirs;
  if (mode == 3) report = "순정 땅 자리 행 +" + dr + " 열 +" + dc + " (" + wm + "판) | 산세 뺌 " + applied + "칸, 이미 순정 " + already + "칸, 맵 확장이 바꾼 칸 " + theirs + "칸은 그대로";
  else report = "순정 땅 자리 행 +" + dr + " 열 +" + dc + " (" + wm + "판) | 산세 넣음 " + applied + "칸, 이미 들어 있음 " + already + "칸, 맵 확장이 바꾼 칸 " + theirs + "칸은 그대로";
  if (mode == 2 || applied == 0) return null;
  // re-compress the touched pieces, then pad back to the exact old size
  int n = me.Len.Count; var pieces = new byte[n][]; long oldSum = 0, newSum = 0;
  for (int k = 0; k < n; k++) { oldSum += me.Len[k]; if (touched.Contains(k)) { pieces[k] = Zlib(me.Raw[k]); if (me.Raw[k].Length != me.CH && k != n - 1) throw new InvalidDataException("조각 크기가 고르지 않음"); } newSum += touched.Contains(k) ? pieces[k].Length : me.Len[k]; }
  long slack = oldSum - newSum;
  if (slack < 0) {
   // squeeze untouched pieces (largest first) - only helps if the other mod packed them loosely
   var order = new List<int>(); for (int k = 0; k < n; k++) if (!touched.Contains(k)) order.Add(k);
   order.Sort(delegate(int a, int b) { return me.Len[b].CompareTo(me.Len[a]); });
   foreach (int k in order) { if (slack >= 0) break; byte[] z = Zlib(me.Piece(k)); if (z.Length < me.Len[k]) { slack += me.Len[k] - z.Length; pieces[k] = z; touched.Add(k); } }
   if (slack < 0) { report += " | 같은 크기 안에 다시 묶이지 않아(" + slack + "바이트) 그대로 둠"; return null; }
  }
  foreach (int k in new List<int>(touched)) {
   if (slack == 0) break;
   int cur = pieces[k].Length; int room = CAP - cur; if (room <= 0) continue;
   int add = (int)Math.Min(slack, room);
   byte[] ex = null; for (int a = add; a >= Math.Max(0, add - 12) && ex == null; a--) { if (a == 0) break; ex = Exact(me.Raw[k], cur + a); }
   if (ex == null) continue; slack -= ex.Length - cur; pieces[k] = ex;
  }
  if (slack != 0) { report += " | 크기를 정확히 맞추지 못해(" + slack + ") 그대로 둠"; return null; }
  var outb = new byte[meCont.Length];
  Buffer.BlockCopy(meCont, 0, outb, 0, me.Hdr); int p = me.Hdr;
  for (int k = 0; k < n; k++) {
   if (pieces[k] == null) { Buffer.BlockCopy(meCont, me.Pos[k] - 4, outb, p, 4 + me.Len[k]); p += 4 + me.Len[k]; }
   else { if (pieces[k].Length > CAP) throw new InvalidDataException("조각이 16000바이트를 넘음"); P32(outb, p, (uint)pieces[k].Length); Buffer.BlockCopy(pieces[k], 0, outb, p + 4, pieces[k].Length); p += 4 + pieces[k].Length; }
  }
  if (p != me.End) throw new InvalidDataException("다시 묶은 길이가 다름");
  Buffer.BlockCopy(meCont, me.End, outb, p, meCont.Length - me.End);
  // verify: every rewritten piece inflates back to the intended data, the rest is untouched
  var chk = new Cont(outb);
  if (chk.Len.Count != n) throw new InvalidDataException("조각 수가 달라짐");
  foreach (int k in touched) { byte[] a = Inflate(outb, chk.Pos[k], chk.Len[k]); byte[] b = me.Raw[k]; if (a.Length != b.Length) throw new InvalidDataException("확인 실패(길이)"); for (int q = 0; q < a.Length; q++) if (a[q] != b[q]) throw new InvalidDataException("확인 실패(내용)"); }
  report += " | 조각 " + touched.Count + "개 다시 묶음, 크기 그대로";
  return outb;
 }
}

"@ -ErrorAction Stop
}catch{$script:MEOK=$false;$script:MEERR=$_.Exception.Message}
$written=@();$res=$null;$script:PB=$null
try{
 if(Get-Process -Name 'SAN14CE_KO' -ErrorAction SilentlyContinue){throw '게임이 켜져 있습니다. 게임을 끄고 다시 실행해 주세요.'}
 $game=Find-GameRoot;$res=Join-Path $game '0002';Set-LogGame $game;Log ('게임 폴더: '+$game)
 $bkRoot=Join-Path $res '_TerrainVisual_Backup'
 $bk=Join-Path $bkRoot 'SeasonMap_Terrain'
 $oldBk=@((Join-Path $bkRoot 'MountainShape_v7'),(Join-Path $bkRoot 'MountainShape_v5'),(Join-Path $bkRoot 'MountainShape_TEST_v4'),(Join-Path $bkRoot 'MountainShape_TEST_v3'),(Join-Path $bkRoot 'MountainShape_TEST_v1'),(Join-Path $bkRoot 'MountainRock_v3'),(Join-Path $bkRoot 'MountainRock_v2'),(Join-Path $bkRoot 'MountainRock_v1'),(Join-Path $bkRoot 'TileCanopy_v3'),(Join-Path $bkRoot 'TilePhoto_v1'),(Join-Path $bkRoot 'TileColor_v2'),(Join-Path $bkRoot 'TileColor_v1'),(Join-Path $bkRoot 'TileColorTest_v3'),(Join-Path $bkRoot 'TileColorTest_v2'),(Join-Path $bkRoot 'TileColorTest_v1'))
 function Find-Original($t){
   $cands=@((Join-Path $bk ($t.Name+'.original.idrk')))
   foreach($d in $oldBk){foreach($n in @($t.Name,($t.Name -replace '^mhk_',''))){$cands += (Join-Path $d ($n+'.original.idrk'))}}
   foreach($c in $cands){if(Test-Path $c){$b=[IO.File]::ReadAllBytes($c);if($b.Length -eq $t.Len -and [MS4]::Sha($b) -eq $t.Orig){return ,$b}}}
   return $null
 }
 # v26k48: the bigger map copies another mod (map expand) appended after our file and pointed the game at - found by the game's own
 # index (FieldEditor4.rdb: offset@size), the same asset id and size as our target. Each: target, file, offset, size, kind, original, season
 function ME-Candidates {
   $script:MEC=New-Object System.Collections.ArrayList;$seen=@{}
   $idxF=Join-Path $res 'FieldEditor4.rdb';$cands=@()
   if(Test-Path -LiteralPath $idxF){$txt=[Text.Encoding]::ASCII.GetString([IO.File]::ReadAllBytes($idxF));foreach($m in [regex]::Matches($txt,'([0-9a-f]{1,9})@([0-9a-f]{1,8})')){$cands += ,@([Convert]::ToInt64($m.Groups[1].Value,16),[Convert]::ToInt64($m.Groups[2].Value,16))}}
   foreach($t in $Targets){
    if(-not(($t.Name -like 'mhk_*') -or ($t.Name -like 'tex_*'))){continue};if($t.Orig -eq $t.New){continue}
    $path=Join-Path $res $t.File;if(-not(Test-Path -LiteralPath $path)){continue}
    $flen=(Get-Item -LiteralPath $path).Length;if($flen -le $t.FileLen){continue}
    $newC=[MS4]::Read($path,$t.Offset,$t.Len);if([MS4]::Sha($newC) -ne $t.New){Log ($t.File+' ['+$t.Name+']: 계절 자리가 새 산이 아니어서 큰 지도 쪽은 건너뜀');continue}
    $origC=Find-Original $t;if($null -eq $origC){Log ($t.File+' ['+$t.Name+']: 원본 백업이 없어 큰 지도 쪽은 건너뜀');continue}
    $aid=[SeasonME]::AssetId($newC);$kind=1;if($t.Name -like 'mhk_*'){$kind=0}
    foreach($c in $cands){$o=$c[0];$n=$c[1]
     if($o -lt $t.FileLen -or ($o+$n) -gt $flen -or $n -lt 64){continue}
     $key=$t.File+'|'+$o;if($seen.ContainsKey($key)){continue}
     $h=[MS4]::Read($path,$o,48);if([SeasonME]::AssetId($h) -ne $aid){continue};if([BitConverter]::ToUInt32($h,8) -ne $n){continue}
     $seen[$key]=1;[void]$script:MEC.Add(@($t,$path,$o,$n,$kind,$origC,$newC))
    }
   }
 }
 if($Uninstall){
   Log '산 사진 무늬 + 산 높이 제거 시작'
   $meMark=Join-Path $game '_SAN14_SeasonMap_terrain_me.txt';$mk=@{}
   if(Test-Path -LiteralPath $meMark){foreach($ln in @(Get-Content -LiteralPath $meMark -Encoding UTF8)){$q=$ln.Trim([char]0xFEFF).Split('|');if($q.Count -ge 6){$mk[$q[0]+'|'+$q[1]]=$q}}}
   # v26k48: the bigger map copy (map expand): our backup when the copy is still exactly what we wrote, otherwise the season cells
   #         are taken back out by content (only cells holding the season value go back to the original; the other mod's own cells stay)
   try{
    if(-not $script:MEOK){throw ('도우미 준비 실패: '+$script:MEERR)}
    ME-Candidates
    foreach($c in $script:MEC){$t=$c[0];$path=$c[1];$o=$c[2];$n=$c[3];$kind=$c[4]
      $cur=[MS4]::Read($path,$o,[int]$n);$curSha=[MS4]::Sha($cur);$key=$t.File+'|'+$o;$bkf=Join-Path $bk ('me_'+$t.Name+'_'+$o+'.idrk')
      if($mk.ContainsKey($key) -and $mk[$key][4] -eq $curSha -and (Test-Path -LiteralPath $bkf)){
        $b=[IO.File]::ReadAllBytes($bkf)
        if($b.Length -eq $n -and (-not $mk[$key][3] -or [MS4]::Sha($b) -eq $mk[$key][3])){[MS4]::Write($path,$o,$b);Log ('큰 지도 자리 원래대로(백업): '+$t.File+' '+$o+' ['+$t.Name+']');continue}
      }
      $rep=$null;$out=[SeasonME]::Merge($cur,$c[5],$c[6],$kind,3,[ref]$rep)
      Log ($t.File+' ['+$t.Name+'] 큰 지도 자리 '+$o+': '+$rep)
      if($null -ne $out){
        [MS4]::Write($path,$o,$out)
        if([MS4]::Sha([MS4]::Read($path,$o,[int]$n)) -ne [MS4]::Sha($out)){[MS4]::Write($path,$o,$cur);throw '큰 지도 자리 쓰기 확인 실패 - 원래대로 되돌림'}
        Log ('큰 지도 자리 산세를 뺐습니다: '+$t.File+' '+$o+' ['+$t.Name+']')
      }
    }
   }catch{Log ('큰 지도(다른 모드) 쪽 산세 빼기는 건너뜀: '+$_.Exception.Message)}
   if(Test-Path -LiteralPath $meMark){Remove-Item -LiteralPath $meMark -Force}
   $todo=@()
   foreach($t in $Targets){
     $path=Join-Path $res $t.File;if(-not(Test-Path $path)){continue}
     $cs=[MS4]::Sha([MS4]::Read($path,$t.Offset,$t.Len));if($cs -eq $t.Orig){continue}
     if($cs -ne $t.New -and -not ($t.Prev.Split('|') -contains $cs)){throw ($t.File+' ['+$t.Name+'] 의 해당 위치가 예상과 다릅니다(다른 패치가 덮었을 수 있음). 아무것도 바꾸지 않았습니다.')}
     $ob=Find-Original $t;if($null -eq $ob){if($t.Optional){Log ($t.File+': 원본 백업이 없어 건너뜀');continue};throw ($t.File+' ['+$t.Name+'] 원본 백업을 찾지 못했습니다. 아무것도 바꾸지 않았습니다.')}
     $todo += ,@($t,$ob)
   }
   foreach($x in $todo){$t=$x[0];$path=Join-Path $res $t.File
     [MS4]::Write($path,$t.Offset,$x[1])
     if([MS4]::Sha([MS4]::Read($path,$t.Offset,$t.Len)) -ne $t.Orig){throw ('복원 확인 실패: '+$t.File)}
     Log ('원래대로 복원: '+$t.File+' ['+$t.Name+']')
   }
   foreach($d in @($bk)+$oldBk){if(Test-Path $d){Remove-Item -Recurse -Force $d}}
   Log '산 사진 무늬와 산 높이가 순정으로 돌아갔습니다.'
   exit 0
 }
 Log '산 사진 무늬 + 산 높이 설치 시작 (산·저산 칸만, 판정 그대로)'
 # 설치 데이터 찾기: 이 폴더와 같은 위치에 풀린 Part2/Part3 폴더까지 자동으로 찾음
 $found=@{}
 $r1=Split-Path -Parent $PackageRoot;$roots=@($PackageRoot,$r1);$r2=Split-Path -Parent $r1;if($r2){$roots+=$r2}
 foreach($searchRoot in $roots){
   if($found.Count -ge $PaySha.Count){break}
   foreach($f in (Get-ChildItem -Path $searchRoot -Recurse -File -Depth 4 -ErrorAction SilentlyContinue | Where-Object { $PaySha.ContainsKey($_.Name) })){if(-not $found.ContainsKey($f.Name)){if([MS4]::Sha([IO.File]::ReadAllBytes($f.FullName)) -eq $PaySha[$f.Name]){$found[$f.Name]=$f.FullName}}}
 }
 foreach($n in $PaySha.Keys){if(-not $found.ContainsKey($n)){throw ('설치 데이터가 없습니다: '+$n+'  (Part ZIP 들을 모두 같은 위치에 풀어 주세요)')}}
 Log '설치 데이터 확인'
 New-Item -ItemType Directory -Force -Path $bk|Out-Null
 $plan=@()
 foreach($t in $Targets){
   $path=Join-Path $res $t.File
   if(-not(Test-Path $path)){if($t.Optional){Log ($t.File+': 없음, 건너뜀');continue};throw ('파일이 없습니다: '+$path)}
   if((Get-Item $path).Length -lt ($t.Offset+$t.Len)){if($t.Optional){Log ($t.File+': 크기가 달라 건너뜀');continue};throw ($t.File+' 크기가 예상보다 작습니다. 아무것도 바꾸지 않았습니다.')}
   $cur=[MS4]::Read($path,$t.Offset,$t.Len);$cs=[MS4]::Sha($cur)
   if($cs -eq $t.New){Log ($t.File+' ['+$t.Name+']: 이미 적용되어 있음');$ob=Find-Original $t;if($null -ne $ob){$bf=Join-Path $bk ($t.Name+'.original.idrk');if(-not(Test-Path $bf)){[IO.File]::WriteAllBytes($bf,$ob)}};continue}
   if($cs -eq $t.Orig){$orig=$cur}
   elseif($t.Prev.Split('|') -contains $cs){$orig=Find-Original $t;if($null -eq $orig){$orig=$cur;Log ($t.File+' ['+$t.Name+']: 이전 산 패치 상태, 원본 백업 없음 -> 지금 상태에서 바로 새 산으로 바꿈 (확인 후 적용)')}else{Log ($t.File+' ['+$t.Name+']: 이전 산 패치 상태 -> 원본 백업에서 순정 데이터를 가져옴')}}
   else{if($t.Optional){Log ($t.File+' ['+$t.Name+']: 순정과 달라 건너뜀(선택 대상)');continue};throw ($t.File+' ['+$t.Name+'] 데이터가 순정이 아닙니다. 아무것도 바꾸지 않았습니다.')}
   if($t.Kind -eq 'MHK'){$nb=[IO.File]::ReadAllBytes($found[$t.Payload])}
   else{
     $nb=New-Object byte[] $t.Len;$p=0
     foreach($sg in $t.Segs.Split(';')){$q=$sg.Split(',')
       if($q[0] -eq 'O'){$o=[int]$q[1];$n=[int]$q[2];[MS4]::Copy($orig,$o,$nb,$p,$n);$p+=$n}
       else{if($null -eq $script:PB){$ms=New-Object IO.MemoryStream;foreach($pn in @('season26.part1','season26.part2','season26.part3')){$pb=[IO.File]::ReadAllBytes($found[$pn]);$ms.Write($pb,0,$pb.Length)};$script:PB=$ms.ToArray()};$o=[int]$q[1];$n=[int]$q[2];[MS4]::Copy($script:PB,$o,$nb,$p,$n);$p+=$n}
     }
     if($p -ne $t.Len){throw ($t.Name+': 조립 크기 불일치')}
   }
   if($nb.Length -ne $t.Len -or [MS4]::Sha($nb) -ne $t.New){throw ($t.File+' ['+$t.Name+']: 새 데이터 조립 확인 실패. 아무것도 바꾸지 않았습니다.')}
   $plan += ,@($t,$cur,$nb,$orig)
 }
 if($plan.Count -eq 0){Log '이미 모두 설치되어 있습니다.'}
 foreach($x in $plan){$t=$x[0];$bf=Join-Path $bk ($t.Name+'.original.idrk');if([MS4]::Sha($x[3]) -ne $t.Orig){Log ($t.File+' ['+$t.Name+']: 원본 백업이 없어 백업은 건너뜀 (되돌릴 때는 Steam 의 게임 파일 무결성 검사)');continue};if(-not(Test-Path $bf)){[IO.File]::WriteAllBytes($bf,$x[3])};if([MS4]::Sha([IO.File]::ReadAllBytes($bf)) -ne $t.Orig){throw '백업 저장 실패.'}}
 Log '원본 백업 완료'
 foreach($x in $plan){$t=$x[0];$path=Join-Path $res $t.File
   $written += ,$x
   [MS4]::Write($path,$t.Offset,$x[2])
   if([MS4]::Sha([MS4]::Read($path,$t.Offset,$t.Len)) -ne $t.New){throw ('적용 확인 실패: '+$t.File)}
   if((Get-Item $path).Length -lt ($t.Offset+$t.Len)){throw '파일 크기가 달라졌습니다.'}
   Log ('적용 완료: '+$t.File+' ['+$t.Name+']')
 }
 Log '산 사진 무늬 + 산 높이 적용 완료'
 # ---- v26k47: 맵 확장 등 다른 모드가 같은 파일 뒤에 붙인 '큰 지도' 높이 자료에도 산세 넣기 (그 모드가 바꾼 칸은 그대로) ----
 try{
  if(-not $script:MEOK){throw ('도우미 준비 실패: '+$script:MEERR)}
  $meMark=Join-Path $game '_SAN14_SeasonMap_terrain_me.txt';$marks=@()
  ME-Candidates
  foreach($c in $script:MEC){$t=$c[0];$path=$c[1];$o=$c[2];$n=$c[3];$kind=$c[4]
    $cur=[MS4]::Read($path,$o,[int]$n);$curSha=[MS4]::Sha($cur)
    $bkf=Join-Path $bk ('me_'+$t.Name+'_'+$o+'.idrk')
    $rep=$null;$out=[SeasonME]::Merge($cur,$c[5],$c[6],$kind,1,[ref]$rep);$pr=[SeasonME]::LastProbes
    Log ($t.File+' ['+$t.Name+'] 큰 지도 자리 '+$o+': '+$rep)
    if($null -ne $out){
      if(-not(Test-Path -LiteralPath $bkf)){[IO.File]::WriteAllBytes($bkf,$cur)};if([MS4]::Sha([IO.File]::ReadAllBytes($bkf)) -ne $curSha){throw '큰 지도 자리 백업 저장 실패'}
      [MS4]::Write($path,$o,$out);$back=[MS4]::Read($path,$o,[int]$n);$nsha=[MS4]::Sha($back)
      if($nsha -ne [MS4]::Sha($out)){[MS4]::Write($path,$o,$cur);throw '큰 지도 자리 쓰기 확인 실패 - 원래대로 되돌림'}
      $marks += ($t.File+'|'+$o+'|'+$n+'|'+$curSha+'|'+$nsha+'|'+$t.Name+'|'+$pr);Log ($t.File+' ['+$t.Name+']: 큰 지도 쪽 산세 적용 완료')
    }elseif([SeasonME]::LastApplied -eq 0 -and [SeasonME]::LastAlready -gt 0){
      $osha='';if(Test-Path -LiteralPath $bkf){$osha=[MS4]::Sha([IO.File]::ReadAllBytes($bkf))}
      $marks += ($t.File+'|'+$o+'|'+$n+'|'+$osha+'|'+$curSha+'|'+$t.Name+'|'+$pr);Log ($t.File+' ['+$t.Name+']: 큰 지도 쪽은 이미 산세가 들어 있음')
    }
  }
  if($marks.Count -gt 0){Set-Content -LiteralPath $meMark -Encoding UTF8 -Value $marks}elseif(Test-Path -LiteralPath $meMark){Remove-Item -LiteralPath $meMark -Force}
 }catch{Log ('큰 지도(다른 모드) 쪽 산세 넣기는 건너뜀: '+$_.Exception.Message)}
}catch{
 Log ('실패: '+$_.Exception.Message)
 if($written.Count -gt 0){foreach($x in $written){try{$t=$x[0];[MS4]::Write((Join-Path $res $t.File),$t.Offset,$x[1]);Log ('설치 전 상태로 되돌림: '+$t.File)}catch{Log ('되돌리기 오류: '+$_.Exception.Message)}}}
 else{Log '게임 파일은 바뀌지 않았습니다.'}
 exit 1
}
