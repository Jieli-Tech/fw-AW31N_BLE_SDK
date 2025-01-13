//bin�ļ�������ݽṹ����

//���ñ���ID = 0x00, 0x0001, len = 0x3C

//key_msg, ID = 606, CFG_KEY_MSG_ID, 0x8189, len  = 0x3C = 60
typedef struct _KEY_MSG {
    u8 short_msg;     //�̰���Ϣ
    u8 long_msg;      //������Ϣ
    u8 hold_msg;      //hold ��Ϣ
    u8 up_msg;        //̧����Ϣ
    u8 double_msg;    //˫����Ϣ
    u8 triple_msg;    //������Ϣ
} KEY_MSG;

typedef struct __KEY_MSG {
    KEY_MSG key_msg[KEY_NUM];  //KEY_NUMָ��user_cfg.lua�ļ����õ�num
};


//audio, ID = 524, CFG_AUDIO_ID, 0x8301, len = 0x04
typedef struct _AUDIO_CONFIG {
    u8 sw;
    u8 max_sys_vol;         //���ϵͳ����
    u8 default_vol;         //����Ĭ������
    u8 tone_vol;            //��ʾ������, Ϊ0����ϵͳ����
} AUDIO_CFG;

//charge, ID = 535, CFG_CHARGE_ID, 0x85C1, 0x05
typedef struct __CHARGE_CONFIG {
    u8 sw;                  //����
	u8 poweron_en;          //֧�ֿ������
    u8 full_v;              //������ѹ
	u8 full_c;              //��������
	u8 charge_c;            //������
} CHARGE_CONFIG;


//��������
//mac: ID: 102, CFG_BT_MAC_ADDR, 0x4BC1, len = 0x06
typedef struct __BLUE_MAC {
    u8 mac[6];
} BLUE_MAC_CFG;


//rf_power: ID: 601, CFG_BT_RF_POWER_ID, 0x8601, len = 0x01
typedef struct __BLUE_RF_POWER {
    u8 rf_power;
} RF_POWER_CFG;

//name: ID: 101, CFG_BT_NAME, 0x4B81, len = 32 * 20 = 0x0294 = 660
typedef struct __BLUE_NAME {
    u8 name[32];
} BLUE_NAME_CFG;

//aec_cfg: ID: 604, CFG_AEC_ID, 0x8681, len = 0x14 = 20
typedef struct __AEC {
    u8 mic_again; 
    u8 dac_again; 
    u16 ndt_max_gain;   //default: 384(0 ~ 2048)
    u16 ndt_min_gain;   //default: 64(0 ~ 2048)
    u16 ndt_fade_gain;  //default: 32(0 ~ 2048)
    u16 nearend_thr;    //default: 50(0 ~ 1024)
    u32 nlp_aggress;    //float, default: 4.0f(0 ~ 35)
    u32 nlp_suppress;   //float, default: 5.0f(0 ~ 10)
    u8 aec_mode;        //diable(0), reduce(1), advance(2)
    u8 ul_eq_en;        //disable(0), enable(1)
} _GNU_PACKED_ AEC;

//tws_pair_code: ID: 602, CFG_TWS_PAIR_CODE_ID, 0x8781, len = 0x02
typedef struct __TWS_PAIR_CODE {
    u16 tws_pair_code;
} TWS_PAIR_CODE_CFG;

//status: ID: 605, CFG_UI_TONE_STATUS_ID, 0x8701, len = 0x1D = 29
typedef struct __STATUS {
    u8 charge_start;    //��ʼ���
    u8 charge_full;     //������
    u8 power_on;        //����
    u8 power_off;       //�ػ�
    u8 lowpower;        //�͵�
    u8 max_vol;         //�������
    u8 phone_in;        //����
    u8 phone_out;       //ȥ��
    u8 phone_activ;     //ͨ����
    u8 bt_init_ok;      //������ʼ�����
    u8 bt_connect_ok;   //�������ӳɹ�
    u8 bt_disconnect;   //�����Ͽ�
    u8 tws_connect_ok;   //�������ӳɹ�
    u8 tws_disconnect;   //�����Ͽ�
} _GNU_PACKED_ STATUS;

typedef struct __STATUS_CONFIG {
    u8 status_sw;
    STATUS led;    //led status
    STATUS tone;   //tone status
} STATUS_CONFIG_CFG;



//MIC����ݷ�����Ҫ���ã�Ӱ��MIC��ƫ�õ�ѹ
//[1]:16K
//[2]:7.5K
//[3]:5.1K 
//[4]:6.8K
//[5]:4.7K
//[6]:3.5K
//[7]:2.9K
//[8]:3K
//[9]:2.5K
//[10]:2.1K
//[11]:1.9K
//[12]:2K
//[13]:1.8K
//[14]:1.6K
//[15]:1.5K
//[16]:1K
//[31]:0.6K
//MIC_TYPE, ID: 537, CFG_MIC_TYPE_ID, 0x89C1, len = 0x03
typedef struct __MIC_TYPE_SEL {
    u8 mic_capless; //MIC����ݷ���
    u8 mic_bias_res;    //MIC����ݷ�����Ҫ���ã�Ӱ��MIC��ƫ�õ�ѹ 1:16K 2:7.5K 3:5.1K 4:6.8K 5:4.7K 6:3.5K 7:2.9K  8:3K  9:2.5K 10:2.1K 11:1.9K  12:2K  13:1.8K 14:1.6K  15:1.5K 16:1K 31:0.6K
    u8 mic_ldo_vsel;//00:2.3v 01:2.5v 10:2.7v 11:3.0v
} MIC_TYPE_SEL;

//LOWPOWER_VOLTAGE, ID: 536, CFG_LOWPOWER_VOLTAGE_ID, 0x, len = 0x04
typedef struct __LOWPOWER_VOLTAGE {
    u16 tone_voltage;       //�͵����ѵ�ѹ, Ĭ��340->3.4V
    u16 power_off_voltage;  //�͵�ػ���ѹ, Ĭ��330->3.3V
} LOWPOWER_VOLTAGE;

//AUTO_OFF_TIME, ID: 603, CFG_AUTO_OFF_TIME_ID, 0x, len = 0x01
typedef struct __AUTO_OFF_TIME {
    u8 auto_off_time;  //û�������Զ��ػ�ʱ��, ��λ������, Ĭ��3����
} AUTO_OFF_TIME;

//LRC_CFG, ID: 607, CFG_LRC_ID, 0x, len = 0x09
typedef struct __LRC_CONFIG {
    u16 lrc_ws_inc;
    u16 lrc_ws_init;
    u16 btosc_ws_inc;
    u16 btosc_ws_init;
    u8 lrc_change_mode;
} _GNU_PACKED_ LRC_CONFIG;

//BLE_CFG, ID: 608, CFG_BLE_ID, 0x, len = 0x0
typedef struct __BLE_CONFIG {
    u8 ble_cfg_en;      //ble������Ч��־
    u8 ble_name[32];    //ble������
    u8 ble_rf_power;    //ble���书��
    u8 ble_addr_en;     //ble ��ַ��Ч��־
    u8 ble_addr[6];     //ble��ַ, ��ble_addr_sw = 1; ����Ч
} _GNU_PACKED_ BLE_CONFIG;

