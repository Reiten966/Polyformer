
#include <AUnit.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include "fixtures_extern.h"
#include <tcUtil.h>

void printMenuItem(MenuItem* menuItem);


RENDERING_CALLBACK_NAME_INVOKE(timeMenuItemTestCb, timeItemRenderFn, "Time", 103, NULL)
RENDERING_CALLBACK_NAME_INVOKE(dateFormattedTestCb, dateItemRenderFn, "Date", 999, NULL)

bool setTimeAndCompareResult(TimeFormattedMenuItem& item, int hr, int min, int sec, int hundreds, const char *expected) {
    char sz[20];
    item.setTime(TimeStorage(hr, min, sec, hundreds));
    item.copyValue(sz, sizeof sz);
    bool success = strncmp(expected, sz, sizeof sz) == 0;
    if(!success) {
        Serial.print("Failed exp="); Serial.print(expected);
        Serial.print(", act="); Serial.println(sz);
    }
    return success;
}

test(testTimeMenuItem12Hr) {
    TimeFormattedMenuItem timeItem12(timeMenuItemTestCb, 111, EDITMODE_TIME_12H);
    assertTrue(setTimeAndCompareResult(timeItem12, 12, 20, 30, 0, "12:20:30PM"));
    assertTrue(setTimeAndCompareResult(timeItem12, 12, 20, 30, 0, "12:20:30PM"));
    assertTrue(setTimeAndCompareResult(timeItem12, 0, 10, 30, 0, "12:10:30AM"));
    assertTrue(setTimeAndCompareResult(timeItem12, 11, 59, 30, 0, "11:59:30AM"));
    assertTrue(setTimeAndCompareResult(timeItem12, 23, 59, 30, 0, "11:59:30PM"));

    TimeFormattedMenuItem timeItem12HHMM(timeMenuItemTestCb, 111, EDITMODE_TIME_12H_HHMM);
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 12, 20, 30, 0, "12:20PM"));
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 12, 20, 30, 0, "12:20PM"));
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 23, 59, 30, 0, "11:59PM"));
}

test(testTimeMenuItem24Hr) {
    TimeFormattedMenuItem timeItem12(timeMenuItemTestCb, 111, EDITMODE_TIME_24H);
    assertTrue(setTimeAndCompareResult(timeItem12, 12, 20, 30, 0, "12:20:30"));
    assertTrue(setTimeAndCompareResult(timeItem12, 0, 10, 59, 0, "00:10:59"));
    assertTrue(setTimeAndCompareResult(timeItem12, 17, 59, 30, 0, "17:59:30"));
    assertTrue(setTimeAndCompareResult(timeItem12, 23, 59, 30, 0, "23:59:30"));

    TimeFormattedMenuItem timeItem12HHMM(timeMenuItemTestCb, 111, EDITMODE_TIME_24H_HHMM);
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 0, 20, 30, 0, "00:20"));
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 12, 20, 30, 0, "12:20"));
    assertTrue(setTimeAndCompareResult(timeItem12HHMM, 23, 59, 30, 0, "23:59"));
}

test(testTimeMenuItemDuration) {
    TimeFormattedMenuItem timeItemDurationSec(timeMenuItemTestCb, 111, EDITMODE_TIME_DURATION_SECONDS);
    assertTrue(setTimeAndCompareResult(timeItemDurationSec, 0, 20, 30, 0, "20:30"));
    assertTrue(setTimeAndCompareResult(timeItemDurationSec, 0, 0, 59, 11, "00:59"));
    assertTrue(setTimeAndCompareResult(timeItemDurationSec, 1, 59, 30, 0, "01:59:30"));

    TimeFormattedMenuItem timeItemDurationHundreds(timeMenuItemTestCb, 111, EDITMODE_TIME_DURATION_HUNDREDS);
    assertTrue(setTimeAndCompareResult(timeItemDurationHundreds, 0, 20, 30, 9, "20:30.09"));
    assertTrue(setTimeAndCompareResult(timeItemDurationHundreds, 0, 0, 59, 99, "00:59.99"));
    assertTrue(setTimeAndCompareResult(timeItemDurationHundreds, 1, 59, 30, 1, "01:59:30.01"));
}

test(testTimeMenuItem24HrEditing) {
    TimeFormattedMenuItem timeItem24(timeMenuItemTestCb, 111, EDITMODE_TIME_HUNDREDS_24H);

    char sz[20];
    timeItem24.copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("Time", sz);

    timeItem24.setTime(TimeStorage(20, 39, 30, 93));
    assertStringCaseEqual("Time", sz);
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("20:39:30.93", sz);

    assertEqual(uint8_t(4), timeItem24.beginMultiEdit());
    assertEqual(23, timeItem24.nextPart());
    assertEqual(20, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(18);

    assertEqual(59, timeItem24.nextPart());
    assertEqual(39, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(30);

    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("18:[30]:30.93", sz);

    assertEqual(59, timeItem24.nextPart());
    assertEqual(30, timeItem24.getPartValueAsInt());

    assertEqual(99, timeItem24.nextPart());
    assertEqual(93, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(10);

    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("18:30:30.[10]", sz);
    timeItem24.stopMultiEdit();

    timeItem24.setTimeFromString("23:44:00.33");
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("23:44:00.33", sz);

    timeItem24.setTimeFromString("8:32");
    timeItem24.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("08:32:00.00", sz);
}

test(dateFormattedMenuItem) {
    DateFormattedMenuItem dateItem(dateFormattedTestCb, 114, nullptr);

    char sz[25];
    dateItem.setDateFromString("2020/08/11");
    dateItem.copyNameToBuffer(sz, sizeof(sz));
    assertStringCaseEqual("Date", sz);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("11/08/2020", sz);

    assertEqual(uint8_t(3), dateItem.beginMultiEdit());
    assertEqual(31, dateItem.nextPart());
    assertEqual(11, dateItem.getPartValueAsInt());
    dateItem.valueChanged(10);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[10]/08/2020", sz);

    assertEqual(12, dateItem.nextPart());
    assertEqual(8, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10/[02]/2020", sz);

    assertEqual(9999, dateItem.nextPart());
    assertEqual(2020, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2018);

    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10/02/[2018]", sz);
    dateItem.stopMultiEdit();

    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10/02/2018", sz);

    DateFormattedMenuItem::setDateSeparator('-');
    DateFormattedMenuItem::setDateFormatStyle(DateFormattedMenuItem::MM_DD_YYYY);

    // now check MM-DD-YYYY
    dateItem.setDate(DateStorage(20, 11, 2019));
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("11-20-2019", sz);

    dateItem.beginMultiEdit();

    assertEqual(12, dateItem.nextPart());
    assertEqual(11, dateItem.getPartValueAsInt());
    dateItem.valueChanged(10);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[10]-20-2019", sz);

    assertEqual(31, dateItem.nextPart());
    assertEqual(20, dateItem.getPartValueAsInt());
    dateItem.valueChanged(14);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10-[14]-2019", sz);

    assertEqual(9999, dateItem.nextPart());
    assertEqual(2019, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2018);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("10-14-[2018]", sz);
    dateItem.stopMultiEdit();

    // lastly check YYYY-MM-DD

    DateFormattedMenuItem::setDateSeparator('*');
    DateFormattedMenuItem::setDateFormatStyle(DateFormattedMenuItem::YYYY_MM_DD);

    dateItem.setDate(DateStorage(20, 11, 2019));
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("2019*11*20", sz);

    dateItem.beginMultiEdit();

    assertEqual(9999, dateItem.nextPart());
    assertEqual(2019, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2018);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("[2018]*11*20", sz);

    assertEqual(12, dateItem.nextPart());
    assertEqual(11, dateItem.getPartValueAsInt());
    dateItem.valueChanged(3);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("2018*[03]*20", sz);

    assertEqual(31, dateItem.nextPart());
    assertEqual(20, dateItem.getPartValueAsInt());
    dateItem.valueChanged(2);
    dateItem.copyValue(sz, sizeof(sz));
    assertStringCaseEqual("2018*03*[02]", sz);
    dateItem.stopMultiEdit();
}

// visible for testing.
int daysForMonth(DateStorage& theDate);

test(testDateLeapYearAndMonthSizes) {

    auto theDate = DateStorage(3, 1, 1929);
    assertEqual(31, daysForMonth(theDate));

    // now do all the leap year test cases
    theDate.month = 2;
    assertEqual(28, daysForMonth(theDate)); // 1929 not a leap year
    theDate.year = 1900;
    assertEqual(28, daysForMonth(theDate)); // 1900 not a leap year. !(YR % 100)
    theDate.year = 1904;
    assertEqual(29, daysForMonth(theDate)); // 1904 is a leap year. YR % 4
    theDate.year = 1600;
    assertEqual(29, daysForMonth(theDate)); // 1600 is a leap year. YR % 400

    theDate.month = 3;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 4;
    assertEqual(30, daysForMonth(theDate));
    theDate.month = 5;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 6;
    assertEqual(30, daysForMonth(theDate));
    theDate.month = 7;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 8;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 9;
    assertEqual(30, daysForMonth(theDate));
    theDate.month = 10;
    assertEqual(31, daysForMonth(theDate));
    theDate.month = 11;
    assertEqual(30, daysForMonth(theDate));
    theDate.month = 12;
    assertEqual(31, daysForMonth(theDate));

}