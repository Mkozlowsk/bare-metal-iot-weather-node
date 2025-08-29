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
- [ ] Przygotowanie basic CI
- [x] Testy toolchaina (potrzebny devboard)
- [x] Sprawdzenie kompilacji testowego projektu

## Faza 1 Podstawowy software (TT: 3 tygodnie)
- [ ] Przygotowywanie rozszerzonego CI (W trakcie pozostalych pkt)
- [ ] Konfiguracja HSI/PLL
- [ ] Implementacja GPIO
- [ ] Implementacja UART z przerwaniami do debugowania
- [ ] Implementacja czujnikow i ich protokolow komunikacyjnych
- [ ] Implementacja LoRa

## Faza 2 Implementacja logiki aplikacji (TT: 3 tygodnie)
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
