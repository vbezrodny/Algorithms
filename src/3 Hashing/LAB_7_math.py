import math

# Данные для M1
m1_data = [7,5,9,9,7,6,8,8,7,9,9,4,5,9,6,9,9,10,10,7,11,7,8,9,6,12,8,9,8,7,10,4,4,7,7,8,6,8,5,5,9,8,7,7,8,6,8,6,8,7,9,9,9,7,10,6,7,7,9,9,8,9,10,9]

# Данные для M2
m2_data = [8,13,6,4,9,5,9,10,3,5,9,4,4,6,9,10,9,6,9,7,7,6,4,6,8,5,10,7,9,9,5,12,10,11,8,7,6,8,7,7,8,11,6,9,4,10,14,7,6,4,6,8,5,7,9,10,5,7,9,6,7,8,6,5,9,6,9]

K = 500
M1, M2 = 64, 67

# Средние
mu1 = K / M1
mu2 = K / M2

print("some text")

# Дисперсия для M1
var1 = sum((x - mu1)**2 for x in m1_data) / M1
std1 = math.sqrt(var1)
cv1 = std1 / mu1

# Дисперсия для M2
var2 = sum((x - mu2)**2 for x in m2_data) / M2
std2 = math.sqrt(var2)
cv2 = std2 / mu2

# Хи-квадрат
chi1 = sum((x - mu1)**2 / mu1 for x in m1_data)
chi2 = sum((x - mu2)**2 / mu2 for x in m2_data)

# Критические значения
crit1 = (M1-1) + 1.96 * math.sqrt(2*(M1-1))
crit2 = (M2-1) + 1.96 * math.sqrt(2*(M2-1))

print(f"M1=64: μ={mu1:.3f}, σ={std1:.3f}, CV={cv1:.3f}, χ²={chi1:.2f}, крит={crit1:.2f}")
print(f"M2=67: μ={mu2:.3f}, σ={std2:.3f}, CV={cv2:.3f}, χ²={chi2:.2f}, крит={crit2:.2f}")

# Медиана M1
sorted_m1 = sorted(m1_data)
median1 = (sorted_m1[31] + sorted_m1[32]) / 2

# Медиана M2
sorted_m2 = sorted(m2_data)
median2 = sorted_m2[33]

# Мода M1
from collections import Counter
mode1 = Counter(m1_data).most_common(1)[0]
mode2 = Counter(m2_data).most_common(1)[0]

print(f"Медиана: M1={median1}, M2={median2}")
print(f"Мода: M1={mode1}, M2={mode2}")