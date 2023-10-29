#include <stdio.h>
#include <string.h>
#include "chess_engine.cpp"


char* input_line();

int main(void)
{
    ChessEngine engine;
    engine.resetBoard();
    char* buf = nullptr;
    char selectedX = -1, selectedY = -1;
    bool loop = true;

    while(loop)
    {
        engine.printBoard(std::cout, selectedX, selectedY);
        printf("g: grab, u: ungrab, m: move, d: delete, q: quit\n");

input:
        delete[] buf;
        printf("[input] ");
        buf = input_line();

        int buf_length = strlen(buf);
        if(buf_length < 1)
        {
            printf("Not enough arguments. Try again.\n");
            goto input;
        }

        switch(buf[0])
        {
            case 'd': // delete
            {
                if(buf_length != 3)
                {
                    printf("Invalid arguments count. Try again.\n");
                    goto input;
                }

                int columnAlphabet = buf[1];
                if(columnAlphabet < 'A' || columnAlphabet > 'H')
                {
                    printf("Not a valid alphabet. Try again. (%c)\n", columnAlphabet);
                    goto input;
                }

                int rowNumber = buf[2] - '0';
                if(rowNumber < 1 || rowNumber > 8)
                {
                    printf("Not a valid number. Try again. (%d)\n", rowNumber);
                    goto input;
                }

                engine.killPieceAt(columnAlphabet - 'A', 8 - rowNumber);
                break;
            }

            // case 's': // set
            //     if(strlen(buf) < 4)
            //     {
            //         printf("Not enough arguments. Try again.\n");
            //         continue;
            //     }

            //     columnAlphabet = buf[1];
            //     if(columnAlphabet < 'A' || columnAlphabet > 'H')
            //     {
            //         printf("Not a valid alphabet. Try again. (%c)\n", columnAlphabet);
            //         continue;
            //     }

            //     rowNumber = buf[2] - '0';
            //     if(rowNumber < 1 || rowNumber > 8)
            //     {
            //         printf("Not a valid number. Try again. (%d)\n", rowNumber);
            //         continue;
            //     }

            //     piece = buf[3];
            //     colorless = getPieceType(piece);
            //     if(colorless != 'P' && colorless != 'R' && colorless != 'N' && colorless != 'B' && colorless != 'K' && colorless != 'Q')
            //     {
            //         printf("Not a valid piece. Try again. (%c)\n", piece);
            //         continue;
            //     }

            //     engine.setPiece(columnAlphabet, rowNumber, piece);
            //     break;

            case 'g': // grab
            {
                if(buf_length != 3)
                {
                    printf("Invalid arguments count. Try again.\n");
                    goto input;
                }

                int columnAlphabet = buf[1];
                if(columnAlphabet < 'A' || columnAlphabet > 'H')
                {
                    printf("Not a valid alphabet. Try again. (%c)\n", columnAlphabet);
                    goto input;
                }

                int rowNumber = buf[2] - '0';
                if(rowNumber < 1 || rowNumber > 8)
                {
                    printf("Not a valid number. Try again. (%d)\n", rowNumber);
                    goto input;
                }

                selectedX = columnAlphabet - 'A';
                selectedY = 8 - rowNumber;
                break;
            }

            case 'u': // ungrab
            {
                selectedX = -1;
                selectedY = -1;
                break;
            }
            
            case 'm': // move
            {
                int srcX, srcY, dstX, dstY;

                if(buf_length == 3)
                {
                    if(selectedX == -1 || selectedY == -1)
                    {
                        printf("Piece not selected. Try again.\n");
                    }

                    int columnAlphabet = buf[1];
                    if(columnAlphabet < 'A' || columnAlphabet > 'H')
                    {
                        printf("Not a valid alphabet. Try again. (%c)\n", columnAlphabet);
                        goto input;
                    }

                    int rowNumber = buf[2] - '0';
                    if(rowNumber < 1 || rowNumber > 8)
                    {
                        printf("Not a valid number. Try again. (%d)\n", rowNumber);
                        goto input;
                    }

                    srcX = selectedX; srcY = selectedY;
                    dstX = columnAlphabet - 'A'; dstY = 8 - rowNumber;
                }
                else if(buf_length == 5)
                {
                    int columnAlphabet = buf[1];
                    if(columnAlphabet < 'A' || columnAlphabet > 'H')
                    {
                        printf("Not a valid alphabet. Try again. (%c)\n", columnAlphabet);
                        goto input;
                    }

                    int rowNumber = buf[2] - '0';
                    if(rowNumber < 1 || rowNumber > 8)
                    {
                        printf("Not a valid number. Try again. (%d)\n", rowNumber);
                        goto input;
                    }
                    
                    int dstColumnAlphabet = buf[3];
                    if(dstColumnAlphabet < 'A' || dstColumnAlphabet > 'H')
                    {
                        printf("Not a valid alphabet. Try again. (%c)\n", dstColumnAlphabet);
                        goto input;
                    }

                    int dstRowNumber = buf[4] - '0';
                    if(dstRowNumber < 1 || dstRowNumber > 8)
                    {
                        printf("Not a valid number. Try again. (%d)\n", dstRowNumber);
                        goto input;
                    }

                    srcX = columnAlphabet - 'A'; srcY = 8 - rowNumber;
                    dstX = dstColumnAlphabet - 'A'; dstY = 8 - dstRowNumber;
                }
                else
                {
                    printf("Invalid arguments count. Try again.\n");
                    goto input;
                }
                
                if(!engine.movePieceTo(srcX, srcY, dstX, dstY))
                {
                    printf("Not a valid movement. Try again.\n");
                    goto input;
                }

                if(buf_length == 3)
                {
                    selectedX = -1;
                    selectedY = -1;
                }
                break;
            }

            case 'q':
            {
                printf("Exiting...\n");
                loop = false;
                break;
            }

            default:
            {
                printf("Not a valid command. Try again.\n");
                goto input;
            }
        }

    }
}

/**
 * 한 줄을 입력받음. 빈 줄은 스킵함.
 * scanf("%s")가 동작이 이상하고 buf 배열 크기를 딱 정해야 한다는 게 빡쳐서 이 함수를 만들게 됨
 */
char* input_line()
{
	int size = 0, alloc_size = 0, ch;
	char *res = nullptr, *old;

	while(1)
	{
		// size가 0인지 확인해야 하기 때문에 getchar()를 while()의 조건식 안에 넣지 못함.
		ch = getchar();
		if(ch == '\n')
		{
			if(size == 0) continue;
			else break;
		}

		size++;
		if(size > alloc_size)
		{
			alloc_size += alloc_size / 2 > 1 ? alloc_size / 2 : 1;
			old = res;
			res = new char[alloc_size];
			for(int i = 0; i < size-1; i++)
			{
				res[i] = old[i];
			}
			if(old != nullptr) delete[] old;
		}
		res[size-1] = ch;
	}

	old = res;
	res = new char[size+1];
	for(int i = 0; i < size; i++)
	{
		res[i] = old[i];
	}
	res[size] = '\0';
	if(old != nullptr) delete[] old;

	return res;
}