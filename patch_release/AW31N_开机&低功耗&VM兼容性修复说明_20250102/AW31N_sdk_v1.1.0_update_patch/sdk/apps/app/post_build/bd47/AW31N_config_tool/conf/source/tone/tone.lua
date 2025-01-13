
local tone_defaults =  {
	{"数字0",     srcdir .. '/tone_file/0.wtg'}, --- 名称，默认文件路径
	{"数字1",     srcdir .. '/tone_file/1.wtg'},
	{"数字2",     srcdir .. '/tone_file/2.wtg'},
	{"数字3",     srcdir .. '/tone_file/3.wtg'},
	{"数字4",     srcdir .. '/tone_file/4.wtg'},
	{"数字5",     srcdir .. '/tone_file/5.wtg'},
	{"数字6",     srcdir .. '/tone_file/6.wtg'},
	{"数字7",     srcdir .. '/tone_file/7.wtg'},
	{"数字8",     srcdir .. '/tone_file/8.wtg'},
	{"数字9",     srcdir .. '/tone_file/9.wtg'},
	{"蓝牙模式",  srcdir .. '/tone_file/bt.wtg'},
	{"连接成功",  srcdir .. '/tone_file/bt_conn.wtg'},
	{"断开连接",  srcdir .. '/tone_file/bt_dconn.wtg'},
    {"对耳连接成功",  srcdir .. '/tone_file/tws_conn.wtg'},
	{"对耳断开连接",  srcdir .. '/tone_file/tws_dconn.wtg'},
	{"低电提示音",srcdir .. '/tone_file/low_power.wtg'},
	{"关机",      srcdir .. '/tone_file/power_off.wtg'},
	{"开机",      srcdir .. '/tone_file/power_on.wtg'},
	{"来电",      srcdir .. '/tone_file/ring.wtg'},
	{"最大音量",  srcdir .. '/tone_file/vol_max.wtg'},
};

-- 设置中文名称对应的文件名
cfg:addToneNameMap{
	{"数字0",           "0"},
	{"数字1",           "1"},
	{"数字2",           "2"},
	{"数字3",           "3"},
	{"数字4",           "4"},
	{"数字5",           "5"},
	{"数字6",           "6"},
	{"数字7",           "7"},
	{"数字8",           "8"},
	{"数字9",           "9"},
	{"蓝牙模式",        "bt"},
	{"连接成功",        "bt_conn"},
	{"断开连接",        "bt_dconn"},
    {"对耳连接成功",    "tws_conn"},
	{"对耳断开连接",    "tws_dconn"},
	{"低电提示音",      "low_power"},
	{"关机",            "power_off"},
	{"开机",            "power_on"},
	{"来电",            "ring"},
	{"最大音量",        "vol_max"},
	-- 添加更多
	-- 如果没有，就会使用文件名
};

-- 设置英文
cfg:addToneNameMapLang("en", {
	{"digit 0",           "0"},
	{"digit 1",           "1"},
	{"digit 2",           "2"},
	{"digit 3",           "3"},
	{"digit 4",           "4"},
	{"digit 5",           "5"},
	{"digit 6",           "6"},
	{"digit 7",           "7"},
	{"digit 8",           "8"},
	{"digit 9",           "9"},
	{"BT mode",           "bt"},
	{"connected",         "bt_conn"},
	{"disconnected",      "bt_dconn"},
    {"tws connected",     "tws_conn"},
	{"tws disconnected",  "tws_dconn"},
	{"low power",         "low_power"},
	{"power off",         "power_off"},
	{"power on",          "power_on"},
	{"ringing",           "ring"},
	{"max volume",        "vol_max"},
	-- 添加更多
	-- 如果没有，就会使用文件名
});

--cfg.projDir路径从source开始
local tone_file_list = cfg:tones("提示音", tone_defaults);

--tone_file_list:setBinInfer(false);
local tv = cfg:tonesView(tone_file_list, {"wts", "aac", "mp3", "wav", "wtg", "msbc", "sbc", "mty", "*"});
if open_by_program == "fw_edit" then
	-- 如果在 fw 编辑中打开，则不要添加提示音等功能
	tv:setFlags{"no-load", "no-add", "no-edit-name", "no-delete"};
end
-- tv:setMainFormat("wtg");
tv:setMainFormatSelector({
	-- {"aac",          "aac"},
	{"wtg (低音质)", "wtg"},
	-- {"保留原来格式", "*"},
	{"wts (低音质)", "wts"},
	{"msbc(中音质)", "msbc"},
	-- {"sbc (高音质)", "sbc"},
	-- {"sin (正弦波)", "sin"},
	-- {"mty (全音质) 请设置输出采样率和码率", "mty"},
	{"保留原来格式", "*"},
});

local tone_list_view = cfg:vBox{
	tv
};

if open_by_program == "create" then
local tone_file_list_layout = cfg:vBox{
	tone_list_view,
	cfg:hBox{
		cfg:stButton("恢复默认提示音", function ()
			cfg:set(tone_file_list, tone_defaults)
		end),
		cfg:stButton("保存提示音文件", function ()
			--local filepath = cfg.projDir .. '/' .. 'tone.cfg';
            local filepath = tone_out_path .. 'tone.cfg';
			cfg:saveTonesFile(filepath, {tone_file_list});
			cfg:saveDumpFile(dump_save_file_path); -- 同时也保存一下dump文件
            if cfg.lang == "zh" then
                cfg:msgBox("info", "tong.cfg 文件保存在路径: " .. tone_out_path);
            else
                cfg:msgBox("info", "tong.cfg file have been saved in " .. tone_out_path);
            end
		end),
	},
};

tone_file_cfg = {}
tone_file_cfg.tone_file_list = tone_file_list;
tone_file_cfg.layout = tone_file_list_layout;

tone_view = {"提示音配置", tone_file_list_layout};

end

cfg:addFirmwareFile("tone",
		"提示音配置",
		5, -- 文件类型，提示音是5
		tone_file_list,
		tone_list_view);

cfg:setTonesExtraDir(cfg.projDir .. '/conf/output/extra_tones'); -- 加载提示音的时候，需要释放tone.cfg的内容，这个是释放的目录

local make_chain_cvt = function (funs)
	return function (frompath, topath)
		if #funs == 1 then return funs[1] end
		
		local fs = {}
		fs[1] = frompath;
		for i = 1, #funs-1 do
			fs[#fs+1] = os.tmpname();
		end
		fs[#fs+1] = topath;
		for i, cvt in ipairs(funs) do
			print(fs[i], '->', fs[i+1]);
			cvt(fs[i], fs[i+1]);
		end
		for i = 1, #funs-1 do
			print('remove: ', fs[i+1]);
			os.remove(fs[i+1]);
		end
		return true
	end
end

-- .wtg format convertor

local wtg_to_wav_format = function(frompath, topath)
	local pcmtmp = os.tmpname();
	cfg:runProg{srcdir .. '/tone/wtg_decode.exe', frompath, pcmtmp};
	cfg:runProg{cfg.rootDir .. "/3rd/ffmpeg.exe", "-f", "s16le", "-v", "8", "-y", "-ar", "8000", "-ac", "1", "-i", pcmtmp, "-f", "wav", topath};
	os.remove(pcmtmp);
	return true; -- good
end;

local to_wtg_format = function (frompath, topath)
	local pcmtmp = os.tmpname();
	local pcmtmp2 = os.tmpname();
	cfg:runProg{cfg.rootDir .. "/3rd/ffmpeg.exe", "-i", frompath,  "-ar", "16000", "-ac", "1" , "-f", "s16le", "-acodec", "pcm_s16le", pcmtmp};
	cfg:runProg{srcdir .. "/tone/wtg_resample.exe", pcmtmp, pcmtmp2};
	cfg:runProg{srcdir .. '/tone/wtg_encode.exe', pcmtmp2, topath, "0", pcmtmp, "200"};
	os.remove(pcmtmp);
	os.remove(pcmtmp2);
	return true; -- good
end; 

-- wtgv2 (wts) format convertor

local wts_to_wav_format = function(frompath, topath)
	cfg:runProg{srcdir .. '/tone/wtgv2_decode.exe', frompath, topath, srcdir .. '/tone/wtgv2_icdftabs.rbin'};
	return true;
end;

local to_wts_format = function(frompath, topath)
	local pcmtmp = os.tmpname();
	local pcmtmp2 = os.tmpname();
    print('save bitrate mode' .. tv:getFormatSaveBitRateMode());
	cfg:runProg{cfg.rootDir .. '/3rd/ffmpeg', '-i', frompath, '-ar', math.floor(tv:getFormatSampleRate() * 2), '-ac', '1', '-f', 's16le', '-acodec', 'pcm_s16le', pcmtmp};
	cfg:runProg{srcdir .. '/tone/wtgv2_resample.exe', pcmtmp, pcmtmp2};
	cfg:runProg{srcdir .. '/tone/wtgv2_encode.exe', pcmtmp2, topath, "o.raw", math.floor(tv:getFormatBitRate()), math.floor(tv:getFormatSampleRate()), '20', tv:getFormatVad(), tv:getFormatSaveBitRateMode()};
	os.remove(pcmtmp);
	os.remove(pcmtmp2);
	return true;
end;

-- .msbc format convertor

local msbc_to_wav_format = function (frompath, topath)
	cfg:runProg{srcdir .. '/tone/msbc_decode.exe', frompath, topath};
	return true; -- good
end;

local to_msbc_format = function (frompath, topath)
	local pcmtmp = os.tmpname();
	cfg:runProg{cfg.rootDir .. "/3rd/ffmpeg.exe", "-i", frompath,  "-ar", "16000", "-ac", "1" , "-f", "wav", pcmtmp};
	cfg:runProg{srcdir .. "/tone/msbc_encode.exe", pcmtmp, topath};
	os.remove(pcmtmp);
	return true; -- good
end;

-- .sbc format convertor

local sbc_to_wav_format = function (frompath, topath)
	cfg:runProg{srcdir .. '/tone/sbc_decode.exe', frompath, topath};
	return true; -- good
end

local to_sbc_format = function (frompath, topath)
	local pcmtmp = os.tmpname();
	cfg:runProg{cfg.rootDir .. "/3rd/ffmpeg.exe", "-i", frompath,  "-ar", "48000", "-ac", "1" , "-f", "wav", pcmtmp};
	cfg:runProg{srcdir .. "/tone/sbc_encode.exe", pcmtmp, topath, "35"};
	os.remove(pcmtmp);
	return true; -- good
end;

-- .aac format convertor
local aac_to_wav_format = function (frompath, topath)
	local pcmtmp = os.tmpname();
	cfg:runProg{srcdir .. '/tone/BTAAC_DECODE.exe', frompath, pcmtmp};

	cfg:runProg{srcdir .. '/tone/BTAAC_DECODE.exe', frompath, topath};
	return true;
end

local to_aac_format = function (frompath, topath)
	local pcmtmp = os.tmpname();
	cfg:runProg{cfg.rootDir .. "/3rd/ffmpeg.exe", "-i", frompath,  "-ar", "32000", "-ac", "1" , "-f", "wav", pcmtmp};
	cfg:runProg{srcdir .. '/tone/BTAAC_ENCODE.exe', '-stdf', '0', '-if', pcmtmp, '-of', topath, '-br', '8000'};
	os.remove(pcmtmp);
	return true;
end;

-- sin
local sin_to_wav_format = function (frompath, topath)
	cfg:runProg{srcdir .. "/tone/sintonecvt.exe", frompath, topath, "16"};
	return true;
end

--[[===================================================================================
==================================== mty 音质选择 =====================================
====================================================================================--]]

-- 一些格式作为目标格式的时候，还需要指定 samplerate 和 bitrate
-- 通过下面的方法设置
tv:setFormatSampleRateList("mty", {
-- {sr_out, {br_out}}
    {8000, {8}},
    {11025, {8, 16, 18, 24}},
    {12000, {16, 18, 24}},

    {16000, {16, 24, 32}},
    {22050, {24, 32, 40, 48, 56}},
    {24000, {24, 32, 40, 48, 56}},

    {32000, {48, 56, 64, 80, 96, 112, 128}},
    {44100, {48, 56, 64, 80, 96, 112, 128}},
    {48000, {48, 56, 64, 80, 96, 112, 128}},
});

--                           def  min   max   step digits
tv:setFormatDisplayVad("wts", {0, 0.00, 0.50, 0.01, 2}); -- 当选择 wts 的时候，显示 vad
tv:setFormatSaveBitRateMode("wts", 1);

tv:setFormatSampleRateRange("wts", {
 -- {sr_out, {val, min, max}},
	{16000, {16000, 5000, 60000}},
	{ 8000, { 8000, 5000, 60000}},
    {11025, {11000, 5000, 60000}},
    {12000, {12000, 5000, 60000}},

    {22050, {20000, 5000, 60000}},
    {24000, {22000, 5000, 60000}},
});


---------------------- mty - >wav, 播放
local mty_to_wav_format = function(frompath, topath)
    cfg:runProg{srcdir .. "/tone/mty2wav.exe", frompath, topath};
	return true; -- good
end

---------------------- mp3 -> mty, 用于存储
local mp3_to_mty_format = function(frompath, topath)
	local mp3tmp = os.tmpname() .. ".mp3";
    cfg:runProg{cfg.rootDir .. "/3rd/ffmpeg.exe", "-i", frompath, "-ar", tv:getFormatSampleRate(), "-ac", "1", "-ab", tv:getFormatBitRate(), mp3tmp};
	cfg:runProg{srcdir .. "/tone/br20_mtyconvert.exe", mp3tmp, "t.raw", topath};
	os.remove(mp3tmp);
	return true; -- good
end

---------------------- wav -> mty, 用于存储
local wav_to_mty_format = function(frompath, topath)
	local mp3tmp = os.tmpname() .. ".mp3";
	cfg:runProg{cfg.rootDir .. "/3rd/ffmpeg.exe", "-i", frompath, "-ar", tv:getFormatSampleRate(), "-ac", "1", "-ab", tv:getFormatBitRate(), mp3tmp};
	cfg:runProg{srcdir .. "/tone/br20_mtyconvert.exe", mp3tmp, "t.raw", topath};
	os.remove(mp3tmp);
	return true; -- good
end

-- 转换成 wav，用于播放
cfg:setFormatConverter("sin",  "wav", sin_to_wav_format);
cfg:setFormatConverter("wtg",  "wav", wtg_to_wav_format);
cfg:setFormatConverter("wts",  "wav", wts_to_wav_format);
cfg:setFormatConverter("msbc", "wav", msbc_to_wav_format);
cfg:setFormatConverter("sbc",  "wav", sbc_to_wav_format);
cfg:setFormatConverter("mty",  "wav", mty_to_wav_format);
cfg:setFormatConverter("aac",  "wav", aac_to_wav_format);

-- 格式互相转换
cfg:setFormatConverter("mp3",  "mty", mp3_to_mty_format);
cfg:setFormatConverter("wav",  "mty", wav_to_mty_format);
cfg:setFormatConverter("sbc",  "mty", make_chain_cvt{ sbc_to_wav_format, wav_to_mty_format});
cfg:setFormatConverter("wtg",  "mty", make_chain_cvt{ wtg_to_wav_format, wav_to_mty_format});
cfg:setFormatConverter("wts",  "mty", make_chain_cvt{ wts_to_wav_format, wav_to_mty_format});
cfg:setFormatConverter("aac",  "mty", make_chain_cvt{ aac_to_wav_format, wav_to_mty_format});
cfg:setFormatConverter("msbc", "mty", make_chain_cvt{msbc_to_wav_format, wav_to_mty_format});

cfg:setFormatConverter("mp3",  "wtg", to_wtg_format);
cfg:setFormatConverter("wav",  "wtg", to_wtg_format);
cfg:setFormatConverter("mty",  "wtg", make_chain_cvt{ mty_to_wav_format, to_wtg_format});
cfg:setFormatConverter("sbc",  "wtg", make_chain_cvt{ sbc_to_wav_format, to_wtg_format});
cfg:setFormatConverter("aac",  "wtg", make_chain_cvt{ aac_to_wav_format, to_wtg_format});
cfg:setFormatConverter("msbc", "wtg", make_chain_cvt{msbc_to_wav_format, to_wtg_format});
cfg:setFormatConverter("wts",  "wtg", make_chain_cvt{ wts_to_wav_format, to_wtg_format});


cfg:setFormatConverter("mp3",  "wts", to_wts_format);
cfg:setFormatConverter("wav",  "wts", to_wts_format);
cfg:setFormatConverter("mty",  "wts", make_chain_cvt{ mty_to_wav_format, to_wts_format});
cfg:setFormatConverter("sbc",  "wts", make_chain_cvt{ sbc_to_wav_format, to_wts_format});
cfg:setFormatConverter("aac",  "wts", make_chain_cvt{ aac_to_wav_format, to_wts_format});
cfg:setFormatConverter("msbc", "wts", make_chain_cvt{msbc_to_wav_format, to_wts_format});
cfg:setFormatConverter("wtg",  "wts", make_chain_cvt{ wtg_to_wav_format, to_wts_format});

cfg:setFormatConverter("mp3", "msbc", to_msbc_format);
cfg:setFormatConverter("wav", "msbc", to_msbc_format);
cfg:setFormatConverter("mty", "msbc", make_chain_cvt{mty_to_wav_format, to_msbc_format});
cfg:setFormatConverter("wtg", "msbc", make_chain_cvt{wtg_to_wav_format, to_msbc_format});
cfg:setFormatConverter("sbc", "msbc", make_chain_cvt{sbc_to_wav_format, to_msbc_format});
cfg:setFormatConverter("aac", "msbc", make_chain_cvt{aac_to_wav_format, to_msbc_format});
cfg:setFormatConverter("wts", "msbc", make_chain_cvt{wts_to_wav_format, to_msbc_format});

cfg:setFormatConverter("mp3",  "sbc", to_sbc_format);
cfg:setFormatConverter("wav",  "sbc", to_sbc_format);
cfg:setFormatConverter("mty",  "sbc", make_chain_cvt{ mty_to_wav_format, to_sbc_format});
cfg:setFormatConverter("wtg",  "sbc", make_chain_cvt{ wtg_to_wav_format, to_sbc_format});
cfg:setFormatConverter("aac",  "sbc", make_chain_cvt{ aac_to_wav_format, to_sbc_format});
cfg:setFormatConverter("msbc", "sbc", make_chain_cvt{msbc_to_wav_format, to_sbc_format});
cfg:setFormatConverter("wts",  "sbc", make_chain_cvt{ wts_to_wav_format, to_sbc_format});

cfg:setFormatConverter("mp3", "aac", to_aac_format);
cfg:setFormatConverter("wav", "aac", to_aac_format);
cfg:setFormatConverter("wtg", "aac", make_chain_cvt{ wtg_to_wav_format, to_aac_format});
cfg:setFormatConverter("sbc", "aac", make_chain_cvt{ sbc_to_wav_format, to_aac_format});
cfg:setFormatConverter("mty", "aac", make_chain_cvt{ mty_to_wav_format, to_aac_format});
cfg:setFormatConverter("msbc","aac", make_chain_cvt{msbc_to_wav_format, to_aac_format});
cfg:setFormatConverter("wts", "aac", make_chain_cvt{ wts_to_wav_format, to_aac_format});
