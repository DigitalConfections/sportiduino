## [1.3.2] - 2018-06-08
### Add check station
- To simplify the procedure for checking the chips before the start, a check station mode was added. Also Since it does not record anything on the chip, participants can be trained in the marking chip on the base station

## [1.3.1] - 2018-06-06
### Speeded up the mark, changes in the settings of the base stations
- Due to optimization of chip reading, the time of the mark is reduced by 40 ms
- Added the ability to limit the maximum possible number of marks on the chip and, thereby, further reduce the time of the mark at the base stations
- In the cleaning station mode, the problem of re-cleaning the chip is solved.

## [1.3.0] - 2018-04-11
### Bug fix and new features
- Bug fix with set settings to statioin
- Add clear station mode
- Sound of the station is amplified by adding transistor to scheme
- Firmware for working with Mifare Classic S50 cards moved to the main repository

## [1.2.0] - 2018-02-17
### Changed the master station communication protocol
- The fixed packet up to 32 bytes is changed to a variable length packet with a maximum length of 32 bytes.
- Added functions to request the firmware version of the master station
- Added the function of transferring the station to automatic regime of cards reading

## [1.1.0] - 2018-02-13
### Delete regime stantion-page, bug fix
- To unify the work with the system, the mode in which the label was written to the page equal to the station number was deleted. A sequential mode is left with the search for the last blank page.
- Fixed the bug that occurs when initializing ntag 213 card
- Fixed the bug thst occurs with transmitting log from base stantion

## [1.0.0] - 2018-02-11
### First stable version