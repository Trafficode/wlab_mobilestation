import struct

# \x01\x10\x21\x32\xa0\xfa\x03\x50\xd4\x12\x00\x00\x00\x00\x00
# \x96\x00\x7c\x00\x44\x01\x20\xff\xe0\x00\x18\x00\x21\x00\x2d
# \x0f\x55\x20\x00\x4d\x01\x70\x01

# sample.version = 0x01;                u8
# sample.ts = 1234000;                  i64
# sample.id[0] = 0x10;                  u8
# sample.id[1] = 0x21;                  u8
# sample.id[2] = 0x32;                  u8
# sample.id[3] = 0xa0;                  u8
# sample.id[4] = 0xfa;                  u8
# sample.id[5] = 0x03;                  u8
# sample.temp_act = 150;                i16
# sample.temp_avg = 124;                i16
# sample.temp_max = 324;                i16
# sample.temp_min = -224;               i16
# sample.temp_max_ts_offset = 224;      i16
# sample.temp_min_ts_offset = 24;       i16
# sample.humidity_act = 33;             u8
# sample.humidity_avg = 45;             u8
# sample.humidity_max = 15;             u8
# sample.humidity_min = 85;             u8
# sample.humidity_max_ts_offset = 32;   i16
# sample.humidity_min_ts_offset = 333;  i16
# sample.battery_voltage = 368;         i16
    
packet = bytearray()
packet.extend(b'\x01\x10\x21\x32\xa0\xfa\x03\x50\xd4\x12\x00\x00\x00\x00\x00')
packet.extend(b'\x96\x00\x7c\x00\x44\x01\x20\xff\xe0\x00\x18\x00\x21\x00\x2d')
packet.extend(b'\x0f\x55\x20\x00\x4d\x01\x70\x01')

packet_dict = {
    "version": packet[0],
    "id": "%02X:%02X:%02X:%02X:%02X:%02X" % (packet[1], packet[2], packet[3], packet[4], packet[5], packet[6]),
    "ts": struct.unpack('<q', packet[7:15])[0],
    "temp_act": struct.unpack('<h', packet[15:17])[0],
    "temp_avg": struct.unpack('<h', packet[17:19])[0],
    "temp_max": struct.unpack('<h', packet[19:21])[0],
    "temp_min": struct.unpack('<h', packet[21:23])[0],
    "temp_max_ts_offset": struct.unpack('<h', packet[23:25])[0],
    "temp_min_ts_offset": struct.unpack('<h', packet[25:27])[0],
    "humidity_act": packet[27],
    "humidity_avg": packet[28],
    "humidity_max": packet[29],
    "humidity_min": packet[30],
    "humidity_max_ts_offset": struct.unpack('<h', packet[31:33])[0],
    "humidity_min_ts_offset": struct.unpack('<h', packet[33:35])[0],
    "battery_voltage": struct.unpack('<h', packet[35:37])[0],
}
print(packet_dict)

# const char *DHTJsonDataTemplate =
#     "{\"UID\":\"%s\",\"TS\":%d,\"SERIE\":{\"Temperature\":"
#     "{\"f_avg\":%s,\"f_act\":%s,\"f_min\":%s,\"f_max\":%s,"
#     "\"i_min_ts\":%u,\"i_max_ts\":%u},"
#     "\"Humidity\":{\"f_avg\":%s,\"f_act\":%s,\"f_min\":%s,"
#     "\"f_max\":%s,\"i_min_ts\":%u,\"i_max_ts\":%u}}}";
packet_wlab_dict = {
    "UID": packet_dict["id"],
    "TS": packet_dict["ts"],
    "SERIE": {
        "Temperature": {
            "f_avg": "%.1f" % (packet_dict["temp_avg"]/10),
            "f_act": "%.1f" % (packet_dict["temp_act"]/10),
            "f_min": "%.1f" % (packet_dict["temp_min"]/10),
            "f_max": "%.1f" % (packet_dict["temp_max"]/10),
            "i_min_ts": packet_dict["ts"] + packet_dict["temp_min_ts_offset"],
            "i_max_ts": packet_dict["ts"] + packet_dict["temp_max_ts_offset"],
        },
        "Humidity": {
            "f_avg": "%.1f" % (packet_dict["temp_avg"]/10),
            "f_act": "%.1f" % (packet_dict["temp_act"]/10),
            "f_min": "%.1f" % (packet_dict["temp_min"]/10),
            "f_max": "%.1f" % (packet_dict["temp_max"]/10),
            "i_min_ts": packet_dict["ts"] + packet_dict["humidity_min_ts_offset"],
            "i_max_ts": packet_dict["ts"] + packet_dict["humidity_max_ts_offset"],
        }
    }
}

print(packet_wlab_dict)