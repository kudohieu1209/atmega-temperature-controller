# Phần cứng & Ghi chú kỹ thuật

## Khối nguồn / vi điều khiển
- MCU: **ATmega16** (kit AVR hỗ trợ cả ATmega32 cùng chân đế).
- Thạch anh ngoài tùy chọn; cấu hình tần số khớp với `delay.h`.

## Khối cảm biến — LM35
- Cảm biến nhiệt độ analog tuyến tính, hệ số **10 mV/°C** (vd. 250 mV = 25°C).
- Chân: `1 = VCC (5V)`, `2 = OUT → PA0/ADC0`, `3 = GND`.
- Chỉ đo dải dương (≥ 0°C); đầu ra trở kháng thấp, nối thẳng vào ADC được.

## Khối ADC (đọc LM35)
- Kênh: **ADC0 (PA0)**.
- Vref: nội bộ **2.56V** (`REFS1=1, REFS0=1`).
- `ADLAR = 1` → kết quả căn trái, đọc 8-bit từ thanh ghi `ADCH`.
- Prescaler: `ADPS2 | ADPS1` = chia 64.
- **Thang đo trực tiếp:** `2.56V / 256 = 10 mV = 1°C/LSB`, trùng đúng hệ số LM35,
  nên `temp = ADCH` cho ra °C mà không cần nhân/chia.

## Khối hiển thị
- LCD ký tự **16x2** (HD44780), thư viện `alcd.h`.
- Khởi tạo: `lcd_init(16)`.
- Ký tự độ `°` = mã `\xDF`.

## Khối nút bấm (active-low, pull-up nội)
| Nút   | Chân | Hành vi                            |
|-------|------|------------------------------------|
| UP    | PB0  | `m++`, vượt 99 → quay về 0          |
| DOWN  | PB1  | `m--`, dưới 0 → quay lên 99         |
| RESET | PB4  | `m = 25` (mặc định)                |

- Chống dội phím: kiểm tra lại sau `20 ms`, chờ nhả nút (`while (btn == 0)`).
- Mỗi lần đổi giá trị → ghi ngay vào EEPROM địa chỉ 0.

## Khối relay đầu ra
| Tải   | Chân | Logic                                   |
|-------|------|-----------------------------------------|
| FAN   | PB2  | `1` khi `temp > m` (quá nóng → làm mát)  |
| LIGHT | PB3  | `1` khi `temp <= m` (đủ mát → sưởi/đèn)  |

> Tùy module relay (kích mức cao hay thấp) có thể cần đảo logic ở `out_relay()`.

## EEPROM
- Địa chỉ `0`: lưu ngưỡng nhiệt độ cài đặt `m`.
- Đọc lúc khởi động; nếu giá trị > 99 (chưa khởi tạo) → đặt mặc định 25.

## Hướng nâng cấp gợi ý
- Thêm vùng trễ (hysteresis) để relay không đóng/ngắt liên tục quanh ngưỡng.
- Lấy trung bình nhiều mẫu ADC để giảm nhiễu trước khi so sánh ngưỡng.
- Tách `lcd_display()` thành hàm riêng để `main()` gọn hơn.
- Đo dải nhiệt độ cao hơn 100°C: chuyển Vref sang AVCC (5V) và đổi công thức quy đổi.
