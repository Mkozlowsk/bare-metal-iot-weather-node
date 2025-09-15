# Szczegolowy plan projektu

## Faza -1 setup dokumentacji (TT: 0-1 dzien)
- [x] Stworzenie roadmapy
- [x] Utworzenie repozytorium GitHub.
- [x] Stworzenie struktury katalogow
- [x] Zebranie dokumentacji

## Faza 0 Przygotowanie srodowiska (TT: 1 dzien + 3dni czekania na devboarda)
- [x] Zakup devboarda
- [x] Przygotowanie szablonu struktury plikow
- [x] Przygotowanie make
- [x] Przygotowanie basic CI
- [x] Testy toolchaina (potrzebny devboard)
- [x] Sprawdzenie kompilacji testowego projektu

## Faza 1 Podstawowy software (TT: 6 tygodni)
- [x] CLK: Konfiguracja HSE
- [x] CLK: Konfiguracja MSI
- [x] CLK: Konfiguracja PLL
- [x] CLK: Konfiguracja LSI
- [x] CLK: Konfiguracje LSE
- [x] CLK: Konfiguracja RTC
- [x] CLK: Dependency bitmap zegarow
- [ ] CLK: Konfiguracja VCO
- [ ] CLK: Kalibracja zegarow
- [x] CLK: Konfiguracja SysCLK
- [ ] CLK: Deinicjalizacje funkcji zegarow
- [ ] CLK: System zabezpieczen zegarow CSE
- [ ] CLK: Obsluga LL bledow i ich logowanie
- [ ] CLK: Zaawansowane zarzadzanie energia, skalowanie napiecia od czestotliwosci, tryby niskopradowe.
- [ ] CLK: Funkcje diagnostyczne zegarow
- [ ] CLK: Preskalery busow AHB, APB
- [ ] CLK: Testy zegarow
- [ ] CI: Przygotowanie rozszerzonego CI:
- [ ] CI: selfhosting hardware
- [ ] CI: symulacja Qemu
- [ ] GPIO: Implementacja GPIO
- [ ] UART: Implementacja UART z przerwaniami do debugowania
- [ ] SERIAL: Implementacja Serial
- [ ] MODULE: Implementacja czujnikow i ich protokolow komunikacyjnych
- [ ] MODULE: Implementacja LoRa

## Faza 2 Implementacja logiki aplikacji (TT: 4 tygodnie)
- [ ] Design protokolu i formatow danych
- [ ] Maszyna stanow
- [ ] Optymalizacja poboru mocy
- [ ] Przygotowanie finalnego CI

# Faza 3 Projekt i produkcja PCB (TT: 2 tygodnie + produkcja i dostawa plytki)
- [ ] Finalny dobor komponentow na podstawie prototypu
- [ ] Schemat elektroniczny i PCB
- [ ] Produkcja i montaz

# Faza 4 Migracja i debugowanie Hardware (TT: 1 tydzien)
- [ ] Dostosowanie kodu do finalnego hardware
- [ ] Wypalenie PCB
- [ ] Debugowanie

# Faza 5 Bootloader OTA (TT: 2 tygodnie)
- [ ] Bootloader OTA

# Faza 6 Implementacja gateway na Raspberry Pi (TT: 1 tydzien)
- [ ] Daemon na RPi

# Faza 7 CI/CD z GitHub Actions i Testy (TT: 2 tygonie)
- [ ] Przygotowanie repo
- [ ] Pipeline dla firmware
- [ ] Pipeline dla daemona w RPi
- [ ] Testy


# TT: ~15 tygodni
