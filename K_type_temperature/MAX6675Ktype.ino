#define _SCK 4
#define _CS  2
#define _SO  6


void max6675_init(){
  pinMode( _SCK, OUTPUT );
  pinMode( _SO, INPUT );
  pinMode( _CS, OUTPUT );
   
  digitalWrite( _CS, HIGH );
}

float max6675_getCelsius() {
   uint16_t t_c = 0;
 
   //初始化溫度轉換
   digitalWrite( _CS, LOW );
   delay(2);
   digitalWrite( _CS, HIGH );
   delay(220);

    // 開始溫度轉換
    digitalWrite( _CS, LOW );
 
    // 15th-bit, dummy bit
    digitalWrite( _SCK, HIGH );
    delay(1);
    digitalWrite( _SCK, LOW );
 
    // 14th - 4th bits, temperature valuw
    for( int i = 11; i >= 0; i-- ) {
       digitalWrite( _SCK, HIGH );
       t_c += digitalRead( _SO ) << i;
       digitalWrite( _SCK, LOW );
    }
 
     // 3th bit, 此位元可以用來判斷 TC 是否損壞或是開路
     // Bit D2 is normally low and goes high when the therometer input is open.
     digitalWrite( _SCK , HIGH );
     is_tc_open = digitalRead( _SO );
     digitalWrite( _SCK, LOW );
 
     // 2nd - 1st bits,
     // D1 is low to provide a device ID for the MAX6675 and bit D0 is three-state.
     for( int i = 1; i >= 0; i-- )
     {
         digitalWrite( _SCK, HIGH );
         delay(1);
         digitalWrite( _SCK, LOW );
     }
 
     // 關閉 MAX6675
     digitalWrite( _CS, HIGH );
 
     return (float)(t_c * 0.25);
}

float max6675_getFahrenheit() {
   float f = max6675_getCelsius();
   if( f != NAN ) {
      return ( f * 9.0/5.0 + 32.0 );
   } else {
      return NAN;
   }
}
