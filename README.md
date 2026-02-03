# OpenCAD
![Gemini_Generated_Image_pfetwrpfetwrpfet](https://github.com/user-attachments/assets/66849253-0f93-4a95-9e38-5eac9aede371)

OpenCAD, CAD (Computer-Aided Design) dosyalarını **okuma, görüntüleme ve işleme** amacıyla geliştirilen açık kaynaklı bir projedir. Projenin temel hedefi, kapalı CAD ekosistemlerine bağımlılığı azaltmak ve geliştiricilere özgür bir altyapı sunmaktır.

---

## 🚀 Özellikler

* 📂 CAD dosyalarını açma ve okuma
* 🔍 Geometri ve metadata analizi
* 🔄 Farklı formatlara dönüştürme (ileride)
* 🧩 Modüler ve genişletilebilir mimari
* 🆓 Açık kaynak ve topluluk odaklı

> Not: Proje aktif geliştirme aşamasındadır. Bazı özellikler deneysel olabilir.

---

## 📁 Desteklenen Dosya Formatları

* `.step` / `.stp`
* `.stl`

⚠️ Kapalı ve tescilli formatlar (örn. `*.sldprt`, `*.sldasm`) **doğrudan** desteklenmez. Bu dosyalar için yalnızca:

* Kullanıcının kendisinin dönüştürdüğü dosyalar
* Açık spesifikasyonu bulunan veriler

kapsam dahilindedir.

---

## 🛠️ Kurulum

```bash
git clone https://github.com/opencad-cam/opencad
```

## ▶️ Kullanım

```bash
run_opencad.bat
```

Örnek kullanım:

```bash
opencad --input model.stp --view
```

---

## ⚖️ Lisans

Bu proje **OpenCad Licanse** ile lisanslanmıştır.

Bu yazılım:

* Tersine mühendislik içermez
* Kapalı kaynak CAD formatlarının şifre çözümünü yapmaz
* Telif hakkı ihlali oluşturacak veri barındırmaz

Detaylar için `LICENSE` dosyasına bakınız.

---

## 🤝 Katkıda Bulunma

Katkılar memnuniyetle karşılanır!

1. Fork'layın
2. Feature branch oluşturun (`feature/yeni-ozellik`)
3. Commit atın
4. Pull Request açın

---

## 📌 Yol Haritası

* [ ] Dosya dönüştürme altyapısı
* [ ] GUI (Qt / Web tabanlı)
* [ ] Topoloji ve parametrik veri desteği
* [ ] Plugin sistemi

---

## 📫 İletişim

Geri bildirim, hata raporu veya öneriler için issue açabilirsiniz.

---

> OpenCAD — özgür tasarım için açık altyapı.
