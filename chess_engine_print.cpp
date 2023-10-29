//
// 여기에 볼 건 없음.
//

#include "chess_engine.h"


void ChessEngine::printBoard(std::ostream &out, int selX, int selY)
{
	char printMatrix[8][8], printCoverTensor[8][8][2];
	for(int y = 0; y < 8; y++) for(int x = 0; x < 8; x++)
	{
		printMatrix[y][x] = '.';
		printCoverTensor[y][x][0] = printCoverTensor[y][x][1] = ' ';
	}

	ChessPiece* selectedPiece = this->getPieceAt(selX, selY);
	if(selectedPiece != nullptr)
	{
		for(int y = 0; y < 8; y++)
		{
			for(int x = 0; x < 8; x++)
			{
				if(!this->isPieceMovableTo(selX, selY, x, y, true, true)) continue;
				printCoverTensor[y][x][0] = '(';
				printCoverTensor[y][x][1] = ')';
			}
		}
	}

    for(int y = 0; y < 8; y++) for(int x = 0; x < 8; x++)
    {
        ChessPiece* piece = this->chessBoard[y][x];
		if(piece == nullptr) continue;

		PieceColor color = piece->color;
		PieceType type = piece->type;

		char printValue = static_cast<char>(type);
		if(color == PieceColor::WHITE) printValue |= 0b00100000;
		printMatrix[y][x] = printValue;

		if(type == PieceType::KING && this->isCheckmate(color) && printCoverTensor[y][x][0] == ' ')
		{
			printCoverTensor[y][x][0] = printCoverTensor[y][x][1] = '!';
		}
    }

	if(0 <= selX && selX < 8 && 0 <= selY && selY < 8)
	{
		printCoverTensor[selY][selX][0] = '[';
		printCoverTensor[selY][selX][1] = ']';
	}

	out << "     A  B  C  D  E  F  G  H  " << std::endl;
	out << "   +------------------------+" << std::endl;
	for(int y = 0; y < 8; y++)
	{
		out << " " << (8-y) << " |";
		for(int x = 0; x < 8; x++)
		{
			out << printCoverTensor[y][x][0] << printMatrix[y][x] << printCoverTensor[y][x][1];
		}
		out << "| " << (8-y) << std::endl;
	}
	out << "   +------------------------+" << std::endl;
	out << "     A  B  C  D  E  F  G  H  " << std::endl;
}