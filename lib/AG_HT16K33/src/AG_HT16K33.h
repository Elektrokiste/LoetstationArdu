#ifndef AG_HT16K33_h_
#define AG_HT16K33_h_

class HT16K33{
    public:
        byte BaseAdress = 0b11100000;



        HT16K33(int Adresse);
        void Init();
        void WriteToDisplay();


    private:
        int Address;

        


};


#endif