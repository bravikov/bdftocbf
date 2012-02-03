//   Файл: bitoperation.h
// Создан: 28 июня 2011
//  Автор: Dmitry Bravikov (bravikov@gmail.com)


// Макросы для работы с битами переменных


// Пример: управление третьим битом переменной var
//   #define FLAG var,3
//
// Управление флагом
//   sb(FLAG); cb(FLAG); // Установить и сбросить флаг
//   gb(FLAG);           // Узнать сброшен или установлен флаг
//   ib(FLAG);           // Инвертировать состояние флага


#ifndef BITOPERATION_H_
#define BITOPERATION_H_


// Базовые макросы. В своих программах НЕ использовать.
#define boShift(n)  (1<<n)
#define boSet(Var,Bit)     (Var |=  boShift(Bit))
#define boClear(Var,Bit)   (Var &= ~boShift(Bit))
#define boInvert(Var,Bit)  (Var ^=  boShift(Bit))
#define boState(Var,Bit)   (Var &   boShift(Bit))


// В своих программах использовать следующие макросы

// Макросы с полными именами
#define setBit(...)     (boSet   (__VA_ARGS__))  // Установка бита
#define clrBit(...)     (boClear (__VA_ARGS__))  // Сброс бита
#define invBit(...)     (boInvert(__VA_ARGS__))  // Инвертирование бита
#define getBit(...)     (boState (__VA_ARGS__))  // Получение состояния бита

// Макросы с сокращенными именами
#define sb(...)         (boSet   (__VA_ARGS__))  // Установка бита
#define cb(...)         (boClear (__VA_ARGS__))  // Сброс бита
#define ib(...)         (boInvert(__VA_ARGS__))  // Инвертирование бита
#define gb(...)         (boState (__VA_ARGS__))  // Получение состояния бита

// TODO (сделать): если макросы существуют, то вывести ошибку


#endif // BITOPERATION_H_

