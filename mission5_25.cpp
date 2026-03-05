/********************************************/
/*     演習５  名簿/リスト構造の内容変更    */
/********************************************/
/*        2026.01.30        n-Iwamoto       */
/*        2026.02.25        n-Iwamoto       */
/********************************************/
/*道具箱を持ってくる*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


/*各種設定*/
#define  MODE_ALL   0x80    /* なんでも入力モード*/
#define  MODE_YN    0x08    /* Y/N入力モード*/
#define  MODE_FLG   0x04    /* 0/1入力モード*/
#define  MODE_ALNUM 0x02    /* 英数字入力モード*/
#define  MODE_DIG   0x01    /* 数字入力モード*/
#define  SEARCH_EQ  0       /* 完全一致検索モード*/
#define  SEARCH_IN  1       /* 挿入位置検索モード*/
#define  CODE_ASC   1       /* 社員コード昇順ソート*/
#define  CODE_DESC  2       /* 社員コード降順ソート*/
#define  NAME_ASC   3       /* 氏名昇順ソート*/
#define  NAME_DESC  4       /* 氏名降順ソート*/
#define  AGE_ASC    5       /* 年齢昇順ソート*/
#define  AGE_DESC   6       /* 年齢降順ソート*/
#define  KEY_LEFT   1000    /* 左矢印入力*/
#define  KEY_RIGHT  1001    /* 右矢印入力*/
#define  KEY_INS    1002    /* insertキー*/


/*移植性のための切り替えスイッチ*/
#ifdef _WIN32
    /*Windows(bcc32)用の設定*/
    #include <dir.h>
    #include <conio.h>
    #define  CLEAR_SCR  "cls"          /*画面リセット*/
    #define  ENT        0x0D           /* Enterキー*/
    #define  BS         0x08           /* Backspaceキー*/
    #define  IS_BS(c)   ((c) == BS)    /*Linuxに合わせコンパイルエラー回避*/

    //Windows用の矢印対応getch()
    int getchArr_win(void){

        /*使用するデータ定義*/
        int c;      /*標準のgetch()で受け取る1文字分*/

        /*実際の処理*/
        c = getch();
        //エスケープ(特殊キーの合図)があったら
        if(c == 0 || c == 0xE0){
            //もう1文字見る
            c = getch();
            if(c == 0x4B){
                //左矢印(←)
                return KEY_LEFT;
            }
            if(c == 0x4D){
                //右矢印(→)
                return KEY_RIGHT;
            }
            if(c == 0x52){
                return KEY_INS;
            }
        }
        //特殊文字じゃない場合はそのまま返す
        return c;
    }
    //getchが来たらgetchArr_winに読み替え
    #define getch() getchArr_win()
#else
    /*Linux用の設定*/
    #include <dirent.h>
    #include <termios.h>
    #include <unistd.h>
    #define  CLEAR_SCR  "clear" /*画面リセット*/
    #define  ENT        0x0A    /* Enterキー*/
    #define  BS         0x7F    /* Backspaceキー*/
    #define  IS_BS(c)   ((c) == 0x7F || (c) == 0x08)    /* DELもBackspaceとして判定*/

    /*Linuxでも動く1文字入力の関数を作る*/
    int getch(void){

        /*使用するデータ定義*/
        struct termios oldt;    /*設定のバックアップ用*/
        struct termios newt;    /*ここで使いたい新しい設定用*/
        int res;                /*入力された文字*/
        int final_char;         /*最終的に入力されたと判断された文字*/
        unsigned char c;        /*キー入力値格納用(矢印入力の際の文字化け防止に符号なし指定)*/
        unsigned char c2;       /*コンボ入力2文字目*/
        unsigned char c3;       /*コンボ入力3文字目*/
        unsigned char c4;       /*コンボ入力4文字目*/

        /*実際の処理*/

        //既存の設定をoldtにコピーして保存しておく
        tcgetattr(STDIN_FILENO, &oldt);
        //同じ内容を書き写し、設定を書き換えていく
        newt = oldt;
        //ICANON（1行入力モード）とECHO（画面に設定する設定）をオフにする
        newt.c_lflag &= ~(ICANON | ECHO);
        //1文字目を読み取る
        newt.c_cc[VMIN] = 1;
        //「0秒」で戻ってくる
        newt.c_cc[VTIME] = 0;        
        //書き換えた設定を反映させる
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        //1文字目が来るまで待機
        //キーボード入力した文字を格納(1バイトだけ読む)
        res = read(0, &c, 1);
        //読めたかどうかの確認
        //1文字読めていたらその文字、読めなかったらエラー
        final_char = (res > 0) ? c : -1;

        //特殊文字の判定
        //エスケープ（コンボの合図）が来た場合
        if(c == 0x1B){

            //2文字目以降の入力は待たない
            newt.c_cc[VMIN] = 0;
            //設定を反映
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);

            //0.01秒待ってから続きを確認
            usleep(10000);

            //2文字目が読み込めて、かつ矢印の場合
            if(read(0, &c2, 1) > 0 && c2 == 0x5B){
                //3文字目を読む
                if(read(0, &c3, 1) > 0){
                    if(c3 == 0x44){
                        //左矢印(←)
                        final_char = KEY_LEFT;
                    }else if(c3 == 0x43){
                        //右矢印(→)
                        final_char = KEY_RIGHT;
                    }else if(c3 == 0x32){
                        //4文字目を読む
                        read(0, &c4, 1);
                        //insertキー
                        final_char = KEY_INS;
                    }
                }
            }
        }
        //最後に設定を元に戻す
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        //入力値(と判断された値)を返す
        return final_char;
    }
    //putch()ときたらputchar()と読み替える
    #define putch(c) putchar(c)
#endif


/*名簿(構造体)の様式*/
struct meibot{
    char            code[6];        /*社員コード*/
    char            name[31];       /*氏名*/
    unsigned char   ge_age;         /*最上位1ビットが性別（1：男 0：女）*/
                                    /*下位7ビットは年齢（最大127歳）*/
    char            telno[16];      /*電話番号*/
    struct meibot   *nextp;         /*次にリストされている構造体へのポインタ*/
};

struct meibot        *head = NULL;  /*先頭にリストされている構造体へのポインタ*/


/*サブルーチンのプロトタイプ宣言*/
void addMeibo(void);                         /*登録*/
void updateMeibo(void);                      /*変更*/
void removeMeibo(void);                      /*削除*/
void showMeibo(void);                        /*一覧表示*/
void saveMeibo(void);                        /*ファイル保存*/
void loadMeibo(void);                        /*ファイル読み込み*/
void searchMeibo(void);                      /*検索*/
void waitMenu(void);                         /*キー入力でメニューに戻る*/
void zeroFill(char *source, char *dest, int length);                  /*ゼロフィル*/
void sortMeibo(struct meibot **sort_board, int count, int mode);      /*名簿並び替え用*/
void printHeader(void);                                               /*名簿の見出し表示用*/
void printMeibo(struct meibot *p);                                    /*名簿1件表示用*/
void freeMeibo(void);                                                 /*全名簿のメモリ開放*/
void attachMeibo(struct meibot *entp);                                /*名簿挿入場所の検索、挿入*/
void detachMeibo(struct meibot *targetp);                             /*名簿の切り離し*/
int  getFileList(char list[20][128]);                                 /*ファイルの一覧表示*/
int  selectMenu(int min, int max);                                    /*メニュー選択*/
int  getstring(char *inbuff, int length, unsigned char mode);         /*文字列入力用*/
struct meibot* search_code(char *target,int mode);                    /*検索・重複確認用*/


/*プログラムスタート*/
int main(void){
    /*使用するデータ定義*/
    int     mode;     /*メニュー選択用*/

    /*これ以降はコンピューターへの指示と制御*/

    //先頭ポインタの初期化
    head = NULL;

    //メニュー表示
    while(1){
        //画面をリセット
        system(CLEAR_SCR);

        printf("***** メニュー *****\n");
        printf("1： 登録\n");
        printf("2： 変更\n");
        printf("3： 削除\n");
        printf("4： 一覧表示\n");
        printf("5： 保存\n");
        printf("6： 読み込み\n");
        printf("7： 検索\n\n");
        printf("9： 終了\n");    
        printf("********************\n");

        //メニュー選択
        mode = selectMenu(1, 7);

        //選択された番号によって各機能を実行
        switch(mode){
            //登録
            case 1:
            //登録処理の呼び出し
            addMeibo();
            //メニューへ
            break;

            //変更
            case 2:
            //変更処理の呼び出し
            updateMeibo();
            //メニューへ
            break;

            //削除
            case 3:
            //削除処理の呼び出し
            removeMeibo();
            //メニューへ
            break;

            //一覧表示
            case 4:
            //一覧表示処理の呼び出し
            showMeibo();
            //メニューへ
            break;

            //保存
            case 5:
            //保存処理の呼び出し
            saveMeibo();
            //メニューへ
            break;

            //読み込み
            case 6:
            //読み込み処理の呼び出し
            loadMeibo();
            //メニューへ
            break;

            //検索
            case 7:
            //検索処理の呼び出し
            searchMeibo();
            //メニューへ
            break;

            //プログラム終了
            case 9:
                //名簿のメモリ開放
                freeMeibo();
                return 0;

            default:
                break;
        }
    }
}


/*メインの処理用サブルーチン*/

/*登録*/
void addMeibo(void){
    /*使用するデータ定義*/
    int         age;         /*年齢入力用*/
    int         gender;      /*性別入力用*/
    int         length;      /*入力された文字列の文字列長-1*/
    int         i;           /*カウンタ変数*/
    char        temp[6];     /*入力値の一時保管用*/
    struct meibot  *entp;    /*新しく接続（登録）する構造体へのポインタ*/
    struct meibot  *listp;   /*接続先（リストの最後）のポインタ*/
    struct meibot  *result;  /*検索結果用*/

    //必要なメモリを確認して確保し、entpに登録
    entp = (struct meibot*)malloc(sizeof(struct meibot));
    //メモリ確保に失敗した場合はエラー
    if(entp == NULL){
        printf("\nメモリ確保に失敗しました。メニュー画面に戻ります。\n");
        return;
    }

    //社員コード入力
    length = 0;
    //１文字以上入力
    while(length == 0){
        printf("\n社員コードを入力してください。\n");
        length = getstring(temp,6,MODE_DIG);
    }
    //ゼロフィル
    zeroFill(temp, entp->code, length);

    //重複チェック
    result = search_code(entp->code, SEARCH_EQ);
    //重複があれば
    if(result != NULL){
        printf("\n社員コード%sは登録済みです。\n",entp->code);
        //メモリ開放
        free(entp);
        //キー入力でメニューに戻る
        waitMenu();
        return;
    }

    //氏名入力
    //１文字以上入力
    length = 0;
    while(length == 0){
        printf("\n氏名を入力してください。\n");
        length = getstring(entp->name,31,MODE_ALNUM);
    }

    //年齢入力
    //１文字以上入力
    length = 0;
    while(length == 0){
        printf("\n年齢を入力してください。\n");
        length = getstring(temp,4,MODE_DIG);
        age = atoi(temp);
        //年齢チェック（0～127歳）
        if(age < 0 || age >127){
            printf("\n%dは入力できません。0～127を入力してください。\n",age);
            length = 0;
        }
    }

    //性別入力
    //１文字以上入力
    length = 0;
    while(length == 0){
        printf("\n性別を入力してください。（１：男性  ０：女性）\n");
        length = getstring(temp,2,MODE_FLG);
        gender = atoi(temp);
    }

    //年齢と性別を名簿の様式に合わせる
    //性別を左に7回ずらし、年齢と合体させる
    entp->ge_age = (unsigned char)((gender << 7) | age);

    //電話番号入力
    //１文字以上入力
    length = 0;
    while(length == 0){
        printf("\n電話番号を入力してください。\n");
        length = getstring(entp->telno,16,MODE_DIG);
    }

    //入力内容確認メッセージ表示
    //見出しの表示
    printHeader();
    //登録内容の表示
    printMeibo(entp);

    //YN入力待ち
    length = 0;
    while(length == 0){
        printf("\nこの名簿を登録しますか？（Y/N）\n");
        length = getstring(temp, 2, MODE_YN);
    }
    //Y入力
    if(temp[0] == 'Y' || temp[0] == 'y'){
        //挿入場所を検索し昇順登録
        attachMeibo(entp);
        printf("\n登録が完了しました。\n");
    //N入力
    }else{
        //確保していたメモリを解放
        free(entp);
        printf("\n登録を中止します。\n");
    }
    //キー入力でメニューに戻る
    waitMenu();
    return;
}


/*変更*/
void updateMeibo(void){
    /*使用するデータ定義*/
    char        temp[6];     /*入力値の一時保管用*/
    char        target[6];   /*社員コード検索用*/
    int         length;      /*入力された文字列の文字列長-1*/
    int         item;        /*選択項目用*/
    int         i;           /*カウンタ変数*/
    int         age;         /*年齢入力用*/
    int         gender;      /*性別入力用*/
    struct meibot  *result;  /*検索結果用*/
    struct meibot  *checkp;  /*重複確認用*/
    struct meibot  *prevp;   /*ひとつ前にリストされている構造体へのポインタ*/

    /*実際の処理*/
    //登録されている名簿があるか確認
    if(head == NULL){
        printf("\n登録されている名簿はありません。\n");
        //キー入力でメニューに戻る
        waitMenu();
        return;
    }

    //変更したい社員コードの入力
    length = 0;
    //１文字以上入力
    while(length == 0){
        printf("\n社員コードを入力してください。\n");
        length = getstring(temp,6,MODE_DIG);
    }

    //ゼロフィル
    zeroFill(temp, target, length);

    //検索
    result = search_code(target, SEARCH_EQ);
    //一致するものがあれば
    if(result != NULL){
        //見つかった名簿1件分の表示
        //見出し表示
        printHeader();
        //名簿１件表示
        printMeibo(result);
        printf("\n上記の名簿情報を変更します。\n");

        //項目を選んで変更
        while(1){
            printf("\n変更したい項目を選んでください。\n");
            printf("1:社員コード 2:氏名 3:年齢 4:性別 5:電話番号 9:終了\n");

            //選択
            item = selectMenu(1,5);

            //終了
            if(item == 9){
                break;
            }

            //変更処理
            switch(item){
                case 1:
                    //社員コード
                    length = 0;
                    //１文字以上入力
                    while(length == 0){
                        printf("\n新しい社員コードを入力してください：");
                        length = getstring(temp,6,MODE_DIG);
                    }

                    //ゼロフィル
                    zeroFill(temp, target, length);

                    //重複チェック
                    checkp = search_code(target, SEARCH_EQ);
                    //重複があれば
                    if(checkp != NULL){
                        printf("\n社員コード%sは登録済みです。\n",target);
                        //キー入力でメニューに戻る
                        waitMenu();
                        return;
                    }

                    //選択した名簿の切り離し
                    detachMeibo(result);
                    //社員コードの書き換え
                    strcpy(result->code, target);
                    //挿入場所を探して昇順登録
                    attachMeibo(result);
                    printf("\n社員コードを変更しました。\n\n");
                    break;

                case 2:
                    //氏名
                    length = 0;
                    //１文字以上入力
                    while(length == 0){
                        printf("\n新しい氏名を入力してください:");
                        length = getstring(result->name,31,MODE_ALNUM);
                    }
                    printf("\n氏名を変更しました。\n\n");
                    break;

                case 3:
                    //年齢
                    length = 0;
                    //１文字以上入力
                    while(length == 0){
                        printf("\n新しい年齢を入力してください:");
                        length = getstring(temp,4,MODE_DIG);
                        age = atoi(temp);
                        //年齢チェック（0～127歳）
                        if(age < 0 || age >127){
                            printf("\n%dは入力できません。0～127を入力してください。\n",age);
                            length = 0;
                        }
                    }
                    //既存データの性別を残し、新しい年齢を上書き
                    result->ge_age = (result->ge_age & 0x80) | (unsigned char)(age & 0x7F);
                    printf("\n年齢を変更しました。\n\n");
                    break;

                case 4:
                    //性別
                    length = 0;
                    //１文字以上入力
                    while(length == 0){
                        printf("\n新しい性別を入力してください。（１：男性  ０：女性）:");
                        length = getstring(temp,2,MODE_FLG);
                        gender = atoi(temp);
                    }
                    //新しく入力された性別を一番左にずらし、今のデータの年齢を残したものと合わせる
                    result->ge_age = (unsigned char)((gender << 7) | (result->ge_age & 0x7F));
                    printf("\n性別を変更しました。\n\n");
                    break;

                case 5:
                    //電話番号
                    length = 0;
                    //１文字以上入力
                    while(length == 0){
                        printf("\n新しい電話番号を入力してください:");
                        length = getstring(result->telno,16,MODE_DIG);
                        printf("\n");
                    }
                    printf("\n電話番号を変更しました。\n\n");
                    break;

                default:
                    printf("\n1～5、または9を入力してください。\n");
                    //変更項目の選択に戻る
                    continue;
            }
            //変更後の名簿を1件表示
            printMeibo(result);
        }

    //入力されたコードと一致する名簿がない場合
    }else{
        printf("\n該当名簿がありません。\n");
    }
    //キー入力でメニューに戻る
    waitMenu();
    return;
}


/*削除*/
void removeMeibo(void){
    /*使用するデータ定義*/
    int         length;      /*入力された文字列の文字列長-1*/
    int         i;           /*カウンタ変数*/
    int         mode;        /*並び替えモード判定*/
    char        temp[6];     /*入力値の一時保管用*/
    char        target[6];   /*社員コード検索用*/
    struct meibot  *result;  /*検索結果用*/

    /*実際の処理*/
    //登録されている名簿があるか確認
    if(head == NULL){
        printf("\n登録されている名簿はありません。\n");
        //キー入力でメニューに戻る
        waitMenu();
        return;
    }

    //モード表示
    while(1){
        printf("\n***** 項目一覧 *****\n");
        printf("1： 1件削除\n");
        printf("2： 全件削除\n\n");
        printf("9： メニューに戻る\n"); 
        printf("*********************\n");

        //モード選択
        mode = selectMenu(1, 2);

        //9が選択されたら
        if(mode == 9){
            //キー入力でメニューに戻る
            waitMenu();
            return;
        }
        //1か2が選択されたら次へ進む
        break;
    }
    
    if(mode == 1){
        //1件削除
        //削除したい社員コードの入力
        length = 0;
        //１文字以上入力
        while(length == 0){
            printf("\n社員コードを入力してください。\n");
            length = getstring(temp,6,MODE_DIG);
        }

        //ゼロフィル
        zeroFill(temp, target, length);

        //社員コードの完全一致検索
        result = search_code(target, SEARCH_EQ);
        //一致するものがあれば
        if(result != NULL){
            //見出し表示
            printHeader();
            //名簿1件分表示
            printMeibo(result);

            //削除前のワンクッション
            printf("\n上記名簿を削除します。よろしいですか？（Y/N）\n");
            //YN入力待ち
            length = 0;
            while(length == 0){
                length = getstring(temp, 2, MODE_YN);
            }
            //Y入力
            if(temp[0] == 'Y' || temp[0] == 'y'){
                //名簿をリストから外す
                detachMeibo(result);
                //メモリ開放
                free(result);
                printf("\n削除しました。\n");
            //N入力
            }else{
                printf("\n削除処理を中止します。\n");
            }

        //入力されたコードと一致する名簿がない場合
        }else{
            printf("\n該当名簿がありません。\n");
        }
    }else{
        //全件削除
        printf("\n既存の名簿データをすべて削除します。よろしいですか？（Y/N）\n");
        //YN入力待ち
        length = 0;
        while(length == 0){
            length = getstring(temp, 2, MODE_YN);
        }
        //Y入力
        if(temp[0] == 'Y' || temp[0] == 'y'){
            freeMeibo();
            printf("\n削除しました。\n");
        //N入力
        }else{
            printf("\n削除処理を中止します。\n");
        }
    }
    //キー入力でメニューに戻る
    waitMenu();
    return;
}


/*一覧表示*/
void showMeibo(void){
/*使用するデータ定義*/
    int         i;                  /*カウンタ変数*/
    int count = 0;                  /*名簿の件数用カウンタ*/
    int display_count = 0;          /*表示した名簿用カウンタ*/
    int mode;                       /*並び替えモード判定*/
    char        temp[6];            /*入力値の一時保管用*/
    struct meibot  *listp;          /*接続先（リストの最後）のポインタ*/
    struct meibot **sort_board;     /*名簿の並び替えをする場所*/

    /*実際の処理*/
    //視線を先頭に持っていく
    listp = head;

    //登録データがない場合
    if(listp == NULL){
        printf("\n登録データはありません。\n\n");

    //データがある場合
    }else{
        //今ある名簿の数を数える
        //視線を先頭へ
        listp = head;
        //名簿がある間繰り返す
        while(listp != NULL){
            //カウントアップ
            count++;
            //次の名簿へ
            listp = listp->nextp;
        }

        //人数分の名簿(の場所のメモ)を置く分のメモリ確保
        sort_board = (struct meibot **)malloc(sizeof(struct meibot *) * count);
        //メモリ確保に失敗した場合はエラー
        if(sort_board == NULL){
            printf("\nメモリ確保に失敗しました。メニュー画面に戻ります。\n");
            return;
        }

        //視線を先頭へ
        listp = head;
        //名簿の数だけ繰り返す
        for(i = 0; i < count; i++){
            //確保した場所(ボード)にメモを貼っていく
            sort_board[i] = listp;
            //視線を次へ
            listp = listp->nextp;
        }

        //モード表示
        while(1){
            printf("\n***** モード一覧 *****\n");
            printf("1： 社員コード昇順\n");
            printf("2： 社員コード降順\n");
            printf("3： 氏名昇順\n");
            printf("4： 氏名降順\n");
            printf("5： 年齢昇順\n");
            printf("6： 年齢降順\n\n");
            printf("9： メニューに戻る\n"); 
            printf("*********************\n");

            //モード選択
            mode = selectMenu(1, 6);

            //9が選択されたら
            if(mode == 9){
                //ボードの片付け
                free(sort_board);
                //キー入力でメニューに戻る
                waitMenu();
                return;
            }
            
            //1～6が選択されたら次へ進む
            break;
        }

        //社員コード昇順以外の場合
        if(mode != CODE_ASC){
            //並び替え
            sortMeibo(sort_board, count, mode);
            printf("\nソートしました。\n");
        }

        //見出しの表示
        printHeader();
        //名簿の数だけ繰り返す
        for(i = 0; i < count; i++){
            //1件分の表示
            printMeibo(sort_board[i]);
            //表示カウンタアップ
            display_count++;

            //10件表示して、かつこれ以降にもデータがある場合
            if(display_count == 10 && (i+1) < count){
                printf("＝＝＝＝＝  何かキーを押せば継続して表示します  ＝＝＝＝＝\n");
                getch();
                //表示カウンタリセット
                display_count =0;
            }
        }
        printf("リストの最後です。\n");
        //ボードの片付け
        free(sort_board);
    }
        //キー入力でメニューに戻る
        waitMenu();
        return;
}


/*ファイル保存*/
void saveMeibo(void){

    /*使用するデータ定義*/
    FILE *fp;                /*ファイルの場所*/
    int         length;      /*入力された文字列の文字列長-1*/
    int         e;           /*エラー判定用*/
    char        temp[6];     /*入力値の一時保管用*/
    char filename[128];      /*ファイル名入力用*/
    struct meibot *listp;    /*今ある名簿を辿る用*/
    struct meibot *nextp;    /*次の人*/

    /*実際の処理*/
    printf("\nファイル名を入力してください。(例：example.txt)\n");
    getstring(filename,128,MODE_ALL);
    //同名ファイルの有無を確認
    fp = fopen(filename,"r");
    //同名ファイルあり
    if(fp != NULL){
        printf("\n%sは存在します。上書きしてよろしいですか？処理を中止しますか？\n",filename);
        printf("(Y:上書き  N:処理中止)\n");
        fclose(fp);

        length = 0;
        //1文字入力
        while(length == 0){
            length = getstring(temp,2,MODE_YN);
        }
        //N入力
        if(temp[0] == 'N' || temp[0] == 'n'){
            printf("\n保存を中止します。");
            //キー入力でメニューに戻る
            waitMenu();
            return;
        }
    }

    //ファイルを開く
    fp = fopen(filename,"w");
    //ファイルを開けなかった場合
    if(fp == NULL){
        printf("\nファイルを開けませんでした。\n");
        //キー入力でメニューに戻る
        waitMenu();
        return;
    }

    //上書き保存処理
    //視線を先頭へ
    listp = head;
    //名簿がある間
    while(listp != NULL){
        //カンマ区切りで名簿情報を書き込み
        e = fprintf(fp,"%s,%s,%d,%s\n",
            listp->code,
            listp->name,
            listp->ge_age,
            listp->telno);
        //書き込み中のエラー判定
        if(e < 0){
            printf("\n書き込みに失敗しました。\n");
            //キー入力でメニューに戻る
            waitMenu();
            return;
        }
        //視線を次へ
        listp = listp->nextp;
    }

    //ファイルを閉じる
    e = fclose(fp);
    //失敗した場合
    if(e == EOF){
        printf("クローズエラーが発生しました。\n");
    }else{
        printf("\n保存が完了しました。\n");
    }
    //キー入力でメニューに戻る
    waitMenu();
    return;
}


/*ファイル読み込み*/
void loadMeibo(void){

    /*使用するデータ定義*/
    FILE *fp;                  /*ファイルの場所*/
    char temp[6];              /*入力値の一時保管用*/
    char filenames[20][128];   /*ファイル名一覧表示呼び出し用*/
    char *target;              /*読み込むファイル*/
    char meibo[256];           /*fgets読み込み用*/
    char temp_code[256];       /*社員コードのチェック用*/
    char temp_name[256];       /*氏名のチェック用*/
    char temp_telno[256];      /*電話番号のチェック用*/
    int  temp_ge_age;          /*年齢と性別のチェック用*/
    int  length;               /*入力された文字列の文字列長-1*/
    int  count = 0;            /*読み込み成功件数カウント用*/
    int  skip = 0;             /*重複スキップ件数カウント用*/
    int  file_count;           /*ファイル件数*/
    int  select;               /*読み込むファイルの番号*/
    int  e;                    /*エラー判定用*/
    struct meibot *listp;      /*今ある名簿を辿る用*/
    struct meibot *nextp;      /*次の人*/
    struct meibot *entp;       /*読み込んできた名簿*/
    struct meibot *result;     /*検索結果用*/

    /*実際の処理*/
    //.txtファイルの一覧を取得して表示
    file_count = getFileList(filenames);

    //ファイルがない場合
    if(file_count == 0){
        //キー入力でメニューに戻る
        waitMenu();
        return;
    }
    //１文字以上入力
    length = 0;
    while(length == 0){
        printf("\n読み込むファイルの番号を入力してください。(99入力でメニューに戻ります)\n");
        //ファイル番号入力
        length = getstring(temp,3,MODE_DIG);
        
        //何も入力されていない場合やり直し
        if(length == 0){
            continue;
        }

        //数値に変換
        select = atoi(temp);

        //99入力でメニューに戻る
        if(select == 99){
            return;
        }

        //存在するファイル番号かどうかのチェック
        if(select >= 1 && select <= file_count){

            //配列からファイル名を取り出す（インデックスにあわせて-1する）
            target = filenames[select-1];

            //ファイルを開く
            fp = fopen(target,"r");

            //エラー処理
            if(fp == NULL){
                printf("\nファイルを開けませんでした。\n");
                //キー入力でメニューに戻る
                waitMenu();
                return;
            }else{
                //次の処理へ
                break;
            }
        //想定されていない入力値の場合
        }else{
            printf("\n1～%d、または99を入力してください。\n",file_count);
        }
        //lengthをリセット
        length = 0;
    }

    //名簿が1件以上登録されている
    if(head != NULL){
        printf("\n既存のデータを破棄しますか？(Y/N)\n");
        length = 0;
        //1文字入力
        while(length == 0){
            length = getstring(temp,2,MODE_YN);
        }
        //Y入力
        if(temp[0] == 'Y' || temp[0] == 'y'){
            //既存の名簿データを全消去
            freeMeibo();
            printf("\n既存のデータを破棄しました。\n");
        }
    }

    //読み込んだファイルを登録
    //データがある間１行ずつ読み込み
    while(fgets(meibo,sizeof(meibo),fp) != NULL){

        //読み込んだ値をカンマ区切りで取り出す
        //文字数などのチェックのため仮置きする
        //データ欠損のチェックも入れる(4つ揃っているか)
        if(sscanf(meibo,"%255[^,],%255[^,],%d,%255[^\n]",
            temp_code,
            temp_name,
            &temp_ge_age,
            temp_telno) != 4){

            printf("\nデータの欠損もしくは読み込み失敗をスキップしました。\n");
            //スキップ件数のカウントアップ
            skip++;
            //次の行の読み込みへ
            continue;
        }

        //文字数チェック(社員コード5、氏名30、電話番号15)
        if(strlen(temp_code) > 5 || strlen(temp_name) > 30 || strlen(temp_telno) > 15){
            printf("\n文字数上限超過のデータをスキップしました。(社員コード：%s)\n",temp_code);
            //スキップ件数のカウントアップ
            skip++;
            //次の行の読み込みへ
            continue;
        }

        //社員コードの重複チェック
        //ゼロフィルし、一時的な箱に入れる
        zeroFill(temp_code, temp, (int)strlen(temp_code));
        if(search_code(temp, SEARCH_EQ) != NULL){
            printf("\n社員コード%sは重複しているためスキップしました。\n",temp);
            //スキップ件数のカウントアップ
            skip++;
            //次の行の読み込みへ
            continue;
        }

        //年齢・性別チェック(正しい形で格納されており、年齢は0～127になっている)
        if((temp_ge_age < 0 || temp_ge_age > 255) || ((temp_ge_age & 0x7F) > 127)){
            printf("\n不正なデータをスキップしました。(社員コード：%s)\n",temp_code);
            //スキップ件数のカウントアップ
            skip++;
            //次の行の読み込みへ
            continue;
        }

        //メモリ確保
        entp = (struct meibot*)malloc(sizeof(struct meibot));
        //エラー処理
        if(entp == NULL){
            printf("\nメモリ確保に失敗しました。メニュー画面に戻ります。\n");
            return;
        }

        //名簿の型にデータを書き写す
        //社員コード
        strcpy(entp->code, temp);
        //氏名
        strcpy(entp->name,temp_name);
        //年齢・性別
        entp->ge_age = (unsigned char)temp_ge_age;
        //電話番号
        strcpy(entp->telno, temp_telno);

        //リストにつなぐ
        attachMeibo(entp);
        //読み込みカウンタアップ
        count++;
    }

    //ファイルを閉じる
    e = fclose(fp);
    if(e == EOF){
        printf("\nクローズエラーが発生しました。\n");
    }else{
        printf("\n読み込みが完了しました。\n(スキップ%d件/%d件中)\n",skip,(count + skip));
    }
    //キー入力でメニューに戻る
    waitMenu();
    return;
}


/*検索*/
void searchMeibo(void){

    /*使用するデータ定義*/
    /*変数*/
    int  i;                 /*カウンタ変数*/
    int  hit;               /*ヒット件数カウント用*/
    int  hit_flg;           /*ヒット判定用フラグ*/
    int  header_flg;        /*見出し表示用フラグ*/
    int  mode;              /*モード判定*/
    char temp[6];           /*入力値の一時保管用*/
    char target_code[6];    /*社員コード検索用*/
    char target_name[31];   /*氏名検索用*/
    char lower_target[31];  /*検索ワードの小文字用*/
    char lower_name[31];    /*名簿氏名の小文字用*/
    struct meibot *listp;   /*今見てる名簿*/

    /*実際の処理*/

    //登録されている名簿があるか確認
    if(head == NULL){
        printf("\n登録されている名簿はありません。\n");
        //キー入力でメニューに戻る
        waitMenu();
        return;
    }

    //モード表示
    while(1){
        printf("\n***** モード一覧 *****\n");
        printf("1： 社員コード検索\n");
        printf("2： 氏名検索\n\n");
        printf("9： メニューに戻る\n"); 
        printf("*********************\n");

        //モード選択
        mode = selectMenu(1, 2);

        //9が選択されたら
        if(mode == 9){
            //キー入力でメニューに戻る
            waitMenu();
            return;
        }
        //1か2が選択されたら次へ進む
        break;
    }

    //ヒット件数のリセット
    hit = 0;

    //検索モードによって入力モードを切り替える
    if(mode == 1){
        //社員コード検索
        printf("\n検索したい社員コードを入力してください。(部分一致検索)\n");
        getstring(target_code, 6, MODE_DIG);
    }else{
        //氏名検索
        printf("\n検索したい氏名を入力してください。(部分一致検索)\n");
        getstring(target_name, 31, MODE_ALNUM);

        //入力された文字をすべて小文字に変換する
        //ヌル文字になるまで1文字ずつ繰り返す
        for(i = 0; target_name[i] != '\0'; i++){
            //小文字変換して配列に格納する(char型が負の値として扱われるのを防ぐためにunsigned charへキャストする)
            lower_target[i] = (char)tolower((unsigned char)target_name[i]);
        }
        //最後にヌル文字を入れる
        lower_target[i] = '\0';
    }

    //検索と結果の表示
    //見出しフラグをリセット
    header_flg = 0;

    //視線をリストの先頭へ
    listp = head;

    //名簿がある間繰り返す
    while(listp != NULL){
        //今見ている名簿が検索条件と合ったか判定する準備
        hit_flg = 0;

        //社員コード検索
        if(mode == 1){
            //社員コードの部分一致あり
            if(strstr(listp->code, target_code) != NULL){
                //ヒット判定
                hit_flg = 1;
            }
        //氏名検索
        }else{
            //名簿に登録されている氏名をすべて小文字に変換する
            //ヌル文字になるまで1文字ずつ繰り返す
            for(i = 0; listp->name[i] != '\0'; i++){
                //小文字変換して配列に格納する
                lower_name[i] = (char)tolower((unsigned char)listp->name[i]);
            }
            //最後にヌル文字を入れる
            lower_name[i] = '\0';
            
            //氏名の部分一致あり(大文字、小文字の区別をしない)
                if(strstr(lower_name, lower_target) != NULL){
                //ヒット判定
                hit_flg = 1;
            }
        }

        //ヒットした
        if(hit_flg == 1){
            //見出しをまだ出していない(初回ヒット)
            if(header_flg == 0){
                //見出しの表示
                printHeader();
                //「見出し表示済み」に切り替え
                header_flg = 1;
            }
            //名簿の１件表示
            printMeibo(listp);
            //件数カウントアップ
            hit++;
        }
        //視線を次へ
        listp = listp->nextp;
    }

    //検索結果
    if(hit == 0){
        printf("\n該当するデータはありませんでした。\n");
    }else{
        printf("\n%d件が見つかりました。\n", hit);
    }
    //メニューに戻る
    waitMenu();
}

/*その他のサブルーチン*/

/*メニューに戻る待機*/
void waitMenu(void){

    /*実際の処理*/
    printf("キー入力でメニューに戻ります。\n\n");
    getch();
}


/*ゼロフィル*/
void zeroFill(char *source, char *dest, int length){

    /*使用するデータ定義*/
    /*引数*/
    /* *source    入力値*/
    /* *dest      格納先*/
    /* length     ヌル文字を含まない入力値の文字数*/

    /*その他の変数*/
    int         zeros;       /*埋めたい0の個数判定用*/
    int         i;           /*カウンタ変数*/

    /*実際の処理*/
    //入力値が5桁に達しているか見る
    zeros = 5 - length;
    //左側を0で埋める
    for(i = 0; i < zeros; i++){
        dest[i] = '0';
    }
    //入力された社員コードを格納する(ヌル文字まで)
    for(i = 0; i <= length; i++){
        dest[zeros + i] = source[i];
    }
}


/*名簿の並び替え*/
void sortMeibo(struct meibot **sort_board, int count, int mode){

    /*使用するデータ定義*/
    /*引数*/
    /* **sort_board     名簿の並び替えをする場所*/
    /* count            名簿の件数*/
    /* mode             並び替えモード判定*/

    /*その他の変数*/
    int  i,j,k;                      /*カウンタ*/
    int  swap;                       /*入れ替え用フラグ*/
    char st1_low[31],st2_low[31];    /*小文字氏名の格納用*/
    struct meibot *temp;             /*名簿入れ替え時の一時保管用*/

    /*実際の処理*/

    //並び替え(バブルソート)
    //何セットやるかの管理
    for(i = 0; i < count - 1; i++){
        //比較回数管理(確定している分は比較しなくていい)
        for(j =0; j < count - 1 - i; j++){
            //入れ替え用フラグのリセット
            swap = 0;
            //隣の名簿と比較して後ろへ追いやる
            //判定モードによって並び替えルールを切り替え
            switch(mode){
                case CODE_ASC:
                    //社員コード昇順
                    if(strcmp(sort_board[j]->code, sort_board[j+1]->code) > 0){
                        swap = 1;
                    }
                    break;
                case CODE_DESC:
                    //社員コード降順
                    if(strcmp(sort_board[j]->code, sort_board[j+1]->code) < 0){
                        swap = 1;
                    }
                    break;
                case NAME_ASC:
                case NAME_DESC:
                    //比較する2つの氏名を小文字に変換
                    //1つ目
                    //ヌル文字まで1文字ずつ見ていく
                    for(k = 0; sort_board[j] ->name[k] != '\0'; k++){
                        //変換し、小文字用配列に格納する
                        st1_low[k] = (char)tolower((unsigned char)sort_board[j]->name[k]);
                    }
                    //最後にヌル文字を入れる
                    st1_low[k] = '\0';
                    //2つ目
                    //ヌル文字まで1文字ずつ見ていく
                    for(k = 0; sort_board[j+1] ->name[k] != '\0'; k++){
                        //変換し、小文字用配列に格納する
                        st2_low[k] = (char)tolower((unsigned char)sort_board[j+1]->name[k]);
                    }
                    //最後にヌル文字を入れる
                    st2_low[k] = '\0';

                    //小文字に変換した同士で比較
                    //氏名昇順
                    if(mode == NAME_ASC && (strcmp(st1_low, st2_low) > 0)){
                        swap = 1;
                    //氏名降順
                    }else if(mode == NAME_DESC && (strcmp(st1_low, st2_low) < 0)){
                        swap = 1;
                    }
                    break;
                case AGE_ASC:
                    //年齢昇順
                    if((sort_board[j]->ge_age & 0x7F) > (sort_board[j+1]->ge_age & 0x7F)){
                        swap = 1;
                    }
                    break;
                case AGE_DESC:
                    //年齢降順
                    if((sort_board[j]->ge_age & 0x7F) < (sort_board[j+1]->ge_age & 0x7F)){
                        swap = 1;
                    }
                    break;
                default:
                    //その他のキーには反応しない
                    break;
            }
            //入れ替えフラグが立っていたら順番を入れ替える
            if(swap){
                //順番が逆なら入れ替える
                //前にあったメモの場所を別の付箋に書き写す
                temp = sort_board[j];
                //前にあったメモに後ろのメモの内容を上書き
                sort_board[j] = sort_board[j+1];
                //後ろのメモに控えておいた付箋の内容を上書き
                sort_board[j+1] = temp;
            }
        }
    }
}


/*名簿の見出し表示*/
void printHeader(void){
    
    /*実際の処理*/
    printf("\n%-10s %-20s %-4s %-6s %-15s\n","社員コード","氏名","年齢","性別","電話番号");
}


/*名簿の1件分表示*/
void printMeibo(struct meibot *p){

    /*使用するデータ定義*/
    /*引数*/
    /* *p        表示したい名簿のポインタ*/

    /*実際の処理*/
    printf("%-10s %-20s %-4d %-6s %-15s\n",
            p->code,
            p->name,
            //年齢：下位7ビットだけを取り出す
            p->ge_age & 0x7F,
            //性別：7つ右にずらして0か1か判定
            (p->ge_age >> 7) ? "男" : "女",
            p->telno);
}


/*既存名簿のメモリ開放*/
void freeMeibo(void){

    /*使用するデータ定義*/
    struct meibot *listp;        /*今見ている名簿*/
    struct meibot *nextp;        /*名簿の「次の人」欄*/

    /*実際の処理*/
    //視線を名簿の先頭へ
    listp = head;
    //登録されている名簿がある間
    while(listp != NULL){
        //「次の人」をメモ
        nextp = listp->nextp;
        //今見ている名簿のメモリを解放
        free(listp);
        //視線をメモした「次の人」へ
        listp = nextp;
    }
    //先頭を空にする
    head = NULL;
}

/*名簿を挿入する場所を探して繋げる*/
void attachMeibo(struct meibot *entp){

    /*使用するデータ定義*/
    /*引数*/
    /* *entp    挿入したい名簿*/
    /*変数*/
    struct meibot *prevp;    /*1つ前の名簿のポインタ*/

    /*実際の処理*/
    //挿入場所の検索
    prevp = search_code(entp->code, SEARCH_IN);
    //先頭への登録の場合
    if(prevp == NULL){
        //新しい名簿の「次の人」欄に、今までの先頭の住所を入れる
        entp->nextp = head;
        //新しい名簿の住所を先頭の場所として登録
        head = entp;
    //途中もしくは最後尾への登録の場合
    }else{
        //新しい名簿の「次の人」欄に、ひとつ前の名簿の「次の人」欄の住所を入れる
        entp->nextp = prevp->nextp;
        //ひとつ前の名簿の「次の人」欄に、新しい名簿の住所を入れる
        prevp->nextp = entp;
    }
}


/*名簿を切り離す*/
void detachMeibo(struct meibot *targetp){

    /*使用するデータ定義*/
    /*引数*/
    /* *targetp    切り離したい名簿*/
    /*変数*/
    struct meibot *prevp;    /*1つ前の名簿のポインタ*/

    /*実際の処理*/
    //リストをつなぎなおすための場所確認
    prevp = search_code(targetp->code, SEARCH_IN);
    //削除したい名簿が先頭の場合
    if(prevp == NULL){
        //「次の人」欄の住所を先頭にする
        head = targetp->nextp;
    //削除したい名簿が途中もしくは最後尾の場合
    }else{
        //ひとつ前の名簿の「次の人」欄を、削除したい名簿の「次の人」欄の内容に書き換える
        prevp->nextp = targetp->nextp;
    }
    //切り離した名簿の「次の人」欄をNULLにする
    targetp->nextp = NULL;
}


/*ファイルの一覧表示*/
int getFileList(char list[20][128]){
    
    /*使用するデータ定義*/
    int count = 0;        /*ファイル数カウント用*/

    printf("\n＝＝＝読み込み可能なファイル一覧(最大20件)＝＝＝\n");

    //Windows
#ifdef _WIN32
    /*使用するデータ定義*/
    struct ffblk fb;       /*ファイル情報を入れる箱*/

    //実際の処理
    //最初の.txtファイルを探す
    if(findfirst("*.txt", &fb, 0) == 0){
        //1つ目
        do{
            //見つけたファイル名を配列にコピー
            strcpy(list[count], fb.ff_name);
            //番号を付けて見つけたファイル名を表示
            printf("%2d : %s\n", count + 1, list[count]);
            //発見カウンタを1増やす
            count++;
        //以降、20件目まで繰り返し
        }while(findnext(&fb) == 0 && count < 20);
    }
    //Linux
#else
    /*使用するデータ定義*/
    DIR *dp;                    /*ディレクトリのどこを見ているかを指すポインタ*/
    struct dirent *entry;       /*ファイル1つ分のデータの場所*/

    //実際の処理
    //今いるディレクトリを開く
    dp = opendir(".");
    //開けた場合
    if(dp != NULL){
        //ファイルがある間繰り返す
        while((entry = readdir(dp)) != NULL && count < 20){
            //.txtのファイルの場合
            if(strstr(entry->d_name, ".txt")){
                //見つけたファイル名を配列にコピー
                strcpy(list[count], entry->d_name);
                //ファイル名表示
                printf("%2d : %s\n", count + 1, list[count]);
                //発見カウンタを1増やす
                count++;
            }
        }
        //ディレクトリを閉じる
        closedir(dp);
    }else{
        //開けなかった場合
        printf("\nディレクトリを開けませんでした。\n");
    }
#endif
    //.txtファイルがなかった場合
    if(count == 0){
        printf("\n対象ファイルが見つかりません。\n");
    }
    printf("＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝\n");
    //見つかった件数を返す
    return count;
}


/*メニュー選択*/
int selectMenu(int min, int max){
    
    /*使用するデータ定義*/
    /*引数*/
    /* min              メニュー項目の最小値*/
    /* max              9を除いたメニュー項目の最大値*/
    /*その他の変数*/
    char temp[6];       /*入力値の受け取り*/
    int  mode;          /*選択された番号*/

    /*実際の処理*/
    while(1){
        printf("\n番号を選択してください：", min, max);
        getstring(temp, 2, MODE_DIG);
        printf("\n");
        
        //9は終了なので別枠
        if(temp[0] == '9'){
            //入力値が9なら9を返す
            return 9;
        }
        
        //入力値を数値に変換
        mode = atoi(temp);
        
        //選択された番号の判定
        if(mode >= min && mode <= max){
            //メニューにある番号ならその値を返す
            return mode;
        }
        //メニューにない番号の場合
        printf("\n%d～%d、または9を入力してください。\n", min, max);
    }
}


/*文字列入力用*/
int getstring(char *inbuff,int length,unsigned char mode){

    /*使用するデータ定義*/
    /*引数*/
    /* *inbuff    入力文字列先頭へのポインタ*/
    /* length     inbuffの長さ（入力可能バイト数＋１）*/
    /* mode       入力モード*/

    /*その他の変数*/
    int i = 0;            /*文字数(入力文字列の最後尾)*/
    int j;                /*カウンタ変数（文字の挿入時、ずらし用）*/
    int c;                  /*入力された文字*/
    int cursor_pos = 0;   /*カーソル位置*/
    int insertFlg = 1;    /*入力モード切替用*/
                          /*1は挿入*/
                          /*0は上書き*/
    /*実際の処理*/

    //画面に反映（入力案内の表示バグ回避）
    fflush(stdout);

    //入力バッファの掃除（表示されないまま進むのを防止。Windowsのみ。）
#ifdef _WIN32
    while(kbhit()){
        getch();
    }
#endif

    while(1){
        //入力値を受け取り、変数cに格納
        c = getch();

        //Backspace判定をBSで統一
        if(IS_BS(c)){
            c = BS;
        }

/* テスト用：何のキーとして認識されているか確認 */
//printf("i:%d, c:0x%02X, BS:0x%02X\n", i, c, BS);

        switch(c){
        //Enterキーが押されたとき
        case ENT:
            //配列にヌル文字を格納
            c ='\0';
            inbuff[i] = c;
            //改行
            putch('\n');
            //カウンタを戻り値として返す
            return i;

        //Backspaceキーが押されたとき
        case BS:

/*BS挙動テスト用*/
//printf("--- BSの処理 iは%d ---\n", i);

            //カーソル位置が先頭でなければ
            if(cursor_pos > 0){

                //データを左に詰める(中抜き処理)
                //消したい文字の場所から、後ろの文字を上書きしていく
                for(j = cursor_pos - 1; j < i - 1; j++){
                    //右隣の文字を自分の場所に持ってくる
                    inbuff[j] = inbuff[j+1];
                }
                //全体の文字数を減らす
                i--;
                //カーソル位置を戻す
                cursor_pos--;

                //画面への反映
                //画面上のカーソルを1つ左へ
                putch(0x08);
                
                //最後尾まで
                for(j = cursor_pos; j < i; j++){
                    //詰めた後の文字を全部書き直し
                    putch(inbuff[j]);
                }
                //余った分空白を上書き
                putch(' ');

                //カーソルをもとの位置（今消した文字の場所）に戻す
                //(書き直した文字数 + 最後の空白)の分だけ
                for(j = 0; j <= (i - cursor_pos); j++){
                    //画面上のカーソルを1つ左へ
                    putch('\b');
                }
                //末尾に蓋(ヌル文字入力)
                inbuff[i] = '\0';
                //画面に反映
                fflush(stdout);
            }
            break;

        //insertキーが押されたとき
        case KEY_INS:
            //1と0を切り替える
            insertFlg = !insertFlg;
            break;

        //左矢印
        case KEY_LEFT:
            //カーソル位置が1つ以上右に進んでいる場合
            if(cursor_pos > 0){
                //画面上のカーソルを1つ左へ
                putch('\b');
                //カーソル位置を1つ左へ
                cursor_pos--;
                //画面に反映(表示バグ回避用)
                fflush(stdout);
            }
            break;

        //右矢印
        case KEY_RIGHT:
            //カーソル位置が入力文字列の途中にある場合
            if(cursor_pos < i){
                //画面上のカーソルを右へ(文字を再描画)
                putch(inbuff[cursor_pos]);
                //カーソル位置を1つ右へ
                cursor_pos++;
                //画面に反映(表示バグ回避用)
                fflush(stdout);
            }
            break;
            
        //それ以外のキーが押されたとき
        default:
            //特殊文字でないかのチェック
            if(c >= 0x20 && c <= 0xFF){
                //入力モードと入力値が一致している場合
                if((mode & MODE_ALL) ||
                    ((mode & MODE_YN) && (c == 'Y' || c == 'y' || c == 'N' || c == 'n'))||
                    ((mode & MODE_FLG) && (c == '0' || c == '1'))||
                    ((mode & MODE_ALNUM) && (isalnum(c) || c == ' ')) ||
                    ((mode & MODE_DIG) && (isdigit(c) || c == ' '))){

                    //insertのモードを確認
                    if(insertFlg){
                        //挿入モード
                        //「今の文字数」が入力上限に達していない場合に実行
                        if(i < length - 1){
                            //iを配列のインデックスとして見て、後ろからずらしていく
                            for(j = i; j > cursor_pos; j--){
                                //1つ左の文字をコピー
                                inbuff[j] = inbuff[j-1];
                            }
                            //入力値をカーソル位置へ、配列の箱の大きさに合わせて格納
                            inbuff[cursor_pos] = (char)c;
                            //文字数カウントアップ
                            i++;
                        }else{
                            //入力上限なら何もしない
                            continue;
                        }
                    }else{
                        //上書きモード
                        //今カーソルがある場所に文字がある場合
                        if(cursor_pos < i){
                            //文字を上書き
                            inbuff[cursor_pos] = (char)c;
                            
                        //末尾の場合の文字数チェック
                        }else if(i < length - 1){
                            //文字入力
                            inbuff[cursor_pos] = (char)c;
                            //文字数カウントアップ
                            i++;
                        }else{
                            continue;
                        }
                    }

                    //画面への再表示表示
                    //今の位置から最後まで
                    for(j = cursor_pos; j < i; j++){
                        //1文字表示
                        putch(inbuff[j]);
                    }
                    //カーソル位置を動かす
                    cursor_pos++;

                    //カーソルを正しい位置へ戻す
                    //(文字数 - カーソル位置)の分だけ
                    for(j = 0; j < (i - cursor_pos); j++){
                        //カーソルを1つ左へ
                        putch('\b');
                    }
                    //末尾に蓋(ヌル文字入力)
                    inbuff[i] = '\0';
                    //画面に反映(表示バグ回避用)
                    fflush(stdout);
                }
            }
            break;
        }
    }
}

/*検索・重複確認用*/
struct meibot* search_code(char *target, int mode){
    /*使用するデータ定義*/
    /*引数*/
    /* char     *target    検索したい社員コードのポインタ*/
    /* int      mode       検索モード*/

    /*その他の変数*/
    int              res;       /*strcmp結果格納用*/
    struct  meibot  *listp;     /*今見ている名簿のポインタ*/
    struct  meibot  *prevp;     /*ひとつ前の名簿のポインタ*/

    /*実際の処理*/

    //視線をリストの先頭へ
    listp = head;
    //「ひとつ前」をリセット
    prevp = NULL;
        //登録されている名簿がある間繰り返す
    while(listp != NULL){
        //検索したい社員コードと、今見ている名簿の社員コードを比較
        res =strcmp(target, listp->code);

        //社員コードが等しい
        if(res == 0){
            //完全一致検索モードなら本人、挿入位置検索モードなら1つ前の名簿を返す
            if(mode == SEARCH_EQ){
                return listp;
            }else{
                return prevp;
            }
        }

        //モードがIN
        if(mode == SEARCH_IN){
            //検索したい社員コードが、今見ている名簿の社員コードより小さい
            if(res < 0){
                //ひとつ前の名簿のアドレスを返す
                //先頭の場合はNULLが入る
                return prevp;
            }
        }
        //前の人を指すポインタを、今見ている名簿へ
        prevp = listp;
        //視線を次の人のアドレスへ移す
        listp = listp->nextp;
    }
    //モードがEQなら「該当名簿なし」、INなら「最後尾」
        return(mode == SEARCH_EQ) ? NULL : prevp;
}