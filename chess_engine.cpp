#include "chess_engine.h"


class PawnPiece : public ChessPiece
{
public:
	bool didMove;

	PawnPiece(int x, int y, PieceColor color)
		: ChessPiece(x, y, PieceType::PAWN, color), didMove(false)
	{}

	bool isMovableTo(ChessEngine &engine, int dstX, int dstY)
	{
		PathState pathState = engine.checkPath(this->x, this->y, dstX, dstY);
		ChessPiece* dstPiece = engine.getPieceAt(dstX, dstY);
		
		if((this->color == PieceColor::BLACK && dstY - this->y == 2) ||
		   (this->color == PieceColor::WHITE && dstY - this->y == -2)) // 2칸 앞 이동
		{
			return didMove == false && pathState == PathState::STRAIGHT_LINE && dstPiece == nullptr;
		}

		if((this->color == PieceColor::BLACK && dstY - this->y == 1) ||
		   (this->color == PieceColor::WHITE && dstY - this->y == -1)) // 1칸 앞/대각선 이동
		{
			switch(pathState)
			{
				case PathState::STRAIGHT_LINE: // 1칸 앞 이동
					return dstPiece == nullptr;

				case PathState::DIAGONAL: // 1칸 대각선 이동
					return dstPiece != nullptr && this->color != dstPiece->color;
			}
		}

		return false;
	}

	bool moveTo(ChessEngine &engine, int dstX, int dstY) override
	{
		// TODO: 끝에 도달했을 때도 구현할 것
		
		bool success = ChessPiece::moveTo(engine, dstX, dstY);
		if(success) this->didMove = true;
		return success;
	}
};


class RookPiece : public ChessPiece
{
public:
	bool didMove;

	RookPiece(int x, int y, PieceColor color)
		: ChessPiece(x, y, PieceType::ROOK, color), didMove(false)
	{}

	bool isMovableTo(ChessEngine &engine, int dstX, int dstY)
	{
		return engine.checkPath(this->x, this->y, dstX, dstY) == PathState::STRAIGHT_LINE;
	}

	bool moveTo(ChessEngine &engine, int dstX, int dstY) override
	{
		bool success = ChessPiece::moveTo(engine, dstX, dstY);
		if(success) this->didMove = true;
		return success;
	}
};


class KnightPiece : public ChessPiece
{
public:
	KnightPiece(int x, int y, PieceColor color)
		: ChessPiece(x, y, PieceType::KNIGHT, color)
	{}

	bool isMovableTo(ChessEngine &engine, int dstX, int dstY)
	{
		if(abs(this->x - dstX) + abs(this->y - dstY) != 3) return false;

		return engine.checkPath(this->x, this->y, dstX, dstY) == PathState::JUMP;
	}
};


class BishopPiece : public ChessPiece
{
public:
	BishopPiece(int x, int y, PieceColor color)
		: ChessPiece(x, y, PieceType::BISHOP, color)
	{}

	bool isMovableTo(ChessEngine &engine, int dstX, int dstY)
	{
		return engine.checkPath(this->x, this->y, dstX, dstY) == PathState::DIAGONAL;
	}
};


class QueenPiece : public ChessPiece
{
public:
	QueenPiece(int x, int y, PieceColor color)
		: ChessPiece(x, y, PieceType::QUEEN, color)
	{}

	bool isMovableTo(ChessEngine &engine, int dstX, int dstY)
	{
		PathState pathState = engine.checkPath(this->x, this->y, dstX, dstY);
		return pathState == PathState::STRAIGHT_LINE || pathState == PathState::DIAGONAL;
	}
};


class KingPiece : public ChessPiece
{
public:
	bool didMove;

	KingPiece(int x, int y, PieceColor color)
		: ChessPiece(x, y, PieceType::KING, color), didMove(false)
	{}

	bool isMovableTo(ChessEngine &engine, int dstX, int dstY)
	{
		// 일반 킹 이동 여부
		if(abs(this->x - dstX) <= 1 && abs(this->y - dstY) <= 1) return true;

		// 캐슬링 확인
		if(this->getCastlingVictim(engine, dstX, dstY) != nullptr) return true;

		return false;
	}

	bool moveTo(ChessEngine &engine, int dstX, int dstY)
	{
		RookPiece* castlingRook = this->getCastlingVictim(engine, dstX, dstY);
		bool success = ChessPiece::moveTo(engine, dstX, dstY);
		if(success)
		{
			this->didMove = true;
			// 캐슬링 조건이 아닐 땐 castlingRook이 nullptr일거라
			// 그냥 실행시켜도 상관 없음
			this->moveRookForCastling(engine, dstX, dstY, castlingRook);
		}
		return success;
	}

private:
	/**
	 * @return 캐슬링할 수 있을 경우 룩의 객체 포인터. 할 수 없을 경우 nullptr
	 */
	RookPiece* getCastlingVictim(ChessEngine &engine, int dstX, int dstY)
	{
		// 킹이 전에 한 번이라도 움직였을 경우 false
		if(this->didMove == true || this->y != dstY) return nullptr;

		int rookX;
		if     (dstX == 2) rookX = 0; // 퀸 사이드 캐슬링
		else if(dstX == 6) rookX = 7; // 킹 사이드 캐슬링
		else return nullptr;

		PathState kingRookPath = engine.checkPath(this->x, this->y, rookX, this->y);
		// 룩 자리와 킹 자리 사이가 막혀있을 경우 false
		if(kingRookPath != PathState::STRAIGHT_LINE) return nullptr;

		ChessPiece* piece = engine.getPieceAt(rookX, this->y);
		// 룩 자리에 룩이 없거나, 룩이 아닌 말이 있거나, 서로 같은 색깔이 아닌 경우
		if(piece == nullptr || piece->type != PieceType::ROOK || this->color != piece->color)
		{
			return nullptr;
		}

		RookPiece* rook = static_cast<RookPiece*>(piece);
		// 룩이 전에 한 번이라도 움직였을 경우 false
		if(rook->didMove) return nullptr;

		return rook;
	}

	void moveRookForCastling(ChessEngine &engine, int kingDstX, int kingDstY, RookPiece* castlingRook)
	{
		// 캐슬링 조건이 아닌 상태에서 이 함수를 실행시킬 수 있기 때문에
		// (이 때 castlingRook으로 nullptr가 들어옴) nullptr 체크를 해야 됨
		if(castlingRook == nullptr) return;

		int rookDstX;
		if     (kingDstX == 2) rookDstX = 3; // 퀸 사이드 캐슬링
		else if(kingDstX == 6) rookDstX = 5; // 킹 사이드 캐슬링
		else return;

		castlingRook->x = rookDstX;
		castlingRook->didMove = true;
	}
};


ChessPiece::ChessPiece(int x_, int y_, PieceType type_, PieceColor color_)
	: x(x_), y(y_), type(type_), color(color_)
{}


ChessPiece::~ChessPiece()
{}


bool ChessPiece::moveTo(ChessEngine &engine, int dstX, int dstY)
{
	ChessPiece* piece = engine.getPieceAt(dstX, dstY);
	if(piece != nullptr)
	{
		PieceColor dstPieceColor = piece->color;
		//
		if(this->color != dstPieceColor)
		{
			engine.killPieceAt(dstX, dstY);
		}
		else return false;
	}
	this->x = dstX;
	this->y = dstY;
	return true;
}


ChessEngine::ChessEngine()
{
	this->resetBoard();
}


ChessEngine::~ChessEngine()
{
	this->clearBoard();
}


void ChessEngine::resetBoard()
{
	this->clearBoard();
	
	// 흑 진영 (뒤)
	this->pieces[ 0] = new RookPiece(0, 0, PieceColor::BLACK);
	this->pieces[ 1] = new KnightPiece(1, 0, PieceColor::BLACK);
	this->pieces[ 2] = new BishopPiece(2, 0, PieceColor::BLACK);
	this->pieces[ 3] = new QueenPiece(3, 0, PieceColor::BLACK);
	this->pieces[ 4] = new KingPiece(4, 0, PieceColor::BLACK);
	this->pieces[ 5] = new BishopPiece(5, 0, PieceColor::BLACK);
	this->pieces[ 6] = new KnightPiece(6, 0, PieceColor::BLACK);
	this->pieces[ 7] = new RookPiece(7, 0, PieceColor::BLACK);

	// 흑 진영 (앞)
	this->pieces[ 8] = new PawnPiece(0, 1, PieceColor::BLACK);
	this->pieces[ 9] = new PawnPiece(1, 1, PieceColor::BLACK);
	this->pieces[10] = new PawnPiece(2, 1, PieceColor::BLACK);
	this->pieces[11] = new PawnPiece(3, 1, PieceColor::BLACK);
	this->pieces[12] = new PawnPiece(4, 1, PieceColor::BLACK);
	this->pieces[13] = new PawnPiece(5, 1, PieceColor::BLACK);
	this->pieces[14] = new PawnPiece(6, 1, PieceColor::BLACK);
	this->pieces[15] = new PawnPiece(7, 1, PieceColor::BLACK);

	// 백 진영 (앞)
	this->pieces[16] = new PawnPiece(0, 6, PieceColor::WHITE);
	this->pieces[17] = new PawnPiece(1, 6, PieceColor::WHITE);
	this->pieces[18] = new PawnPiece(2, 6, PieceColor::WHITE);
	this->pieces[19] = new PawnPiece(3, 6, PieceColor::WHITE);
	this->pieces[20] = new PawnPiece(4, 6, PieceColor::WHITE);
	this->pieces[21] = new PawnPiece(5, 6, PieceColor::WHITE);
	this->pieces[22] = new PawnPiece(6, 6, PieceColor::WHITE);
	this->pieces[23] = new PawnPiece(7, 6, PieceColor::WHITE);
	
	// 백 진영 (뒤)
	this->pieces[24] = new RookPiece(0, 7, PieceColor::WHITE);
	this->pieces[25] = new KnightPiece(1, 7, PieceColor::WHITE);
	this->pieces[26] = new BishopPiece(2, 7, PieceColor::WHITE);
	this->pieces[27] = new QueenPiece(3, 7, PieceColor::WHITE);
	this->pieces[28] = new KingPiece(4, 7, PieceColor::WHITE);
	this->pieces[29] = new BishopPiece(5, 7, PieceColor::WHITE);
	this->pieces[30] = new KnightPiece(6, 7, PieceColor::WHITE);
	this->pieces[31] = new RookPiece(7, 7, PieceColor::WHITE);

	this->chessTurn = PieceColor::WHITE;
}


void ChessEngine::clearBoard()
{
	for(ChessPiece* piece : this->pieces)
	{
		delete piece;
		piece = nullptr;
	}
}


ChessPiece* ChessEngine::getPieceAt(int x, int y)
{
	for(ChessPiece* piece : this->pieces)
	{
		if(piece == nullptr) continue;
		if(piece->x == x && piece->y == y) return piece;
	}
	return nullptr;
}


ChessPiece* ChessEngine::findPiece(PieceType type, PieceColor color)
{
	for(ChessPiece* piece : this->pieces)
	{
		if(piece == nullptr) continue;
		if(piece->type == type && piece->color == color) return piece;
	}
	return nullptr;
}


void ChessEngine::killPieceAt(int x, int y)
{
	for(ChessPiece* &piece : this->pieces)
	{
		if(piece == nullptr) continue;
		if(piece->x != x || piece->y != y) continue;

		delete piece;
		piece = nullptr;
	}
}


bool ChessEngine::isPieceMovableTo(int srcX, int srcY, int dstX, int dstY)
{
	if(srcX == dstX && srcY == dstY) return false; // src == dst일 때 false
	if(srcX < 0 || 8 <= srcX || srcY < 0 || 8 <= srcY) return false; // src가 체스 판 밖일 때 false
	if(dstX < 0 || 8 <= dstX || dstY < 0 || 8 <= dstY) return false; // dst가 체스 판 밖일 때 false

	ChessPiece* srcPiece = this->getPieceAt(srcX, srcY);
	// src에 체스 말이 없다면 false
	if(srcPiece == nullptr) return false;

	// 현재 턴과 맞는지 확인
	if(srcPiece->color != this->chessTurn) return false;
	
	ChessPiece* dstPiece = this->getPieceAt(dstX, dstY);
	// dst에 체스 말이 있는데 src와 같은 색깔이라면 false
	if(dstPiece != nullptr && dstPiece->color == srcPiece->color) return false;

	// TODO: 체크메이트 확인할 것

	return srcPiece->isMovableTo(*this, dstX, dstY);
}


/**
 * (srcX, srcY)와 (dstX, dstY) 사이의 길이 무슨 길인지 알려주는 함수.
 * @return PathState::STRAIGHT_LINE  x축, 또는 y축과 평행한 직선 경로일 때
 *         PathState::DIAGONAL       대각선 경로일 때
 *         PathState::BLOCKED        직선 또는 대각선 경로이지만 중간이 가로막혀 있을 때
 *         PathState::JUMP           직선 또는 대각선 경로도 아닐 때
 */
PathState ChessEngine::checkPath(int srcX, int srcY, int dstX, int dstY)
{
	if(srcY == dstY) // x축과 평행한 직선 경로일 때
	{
		for(int x = min(srcX, dstX) + 1; x < max(srcX, dstX); x++)
		{
			// 사이에 장애물이 있으면 BLOCKED
			if(this->getPieceAt(x, srcY) != nullptr) return PathState::BLOCKED;
		}
		return PathState::STRAIGHT_LINE;
	}
	else if(srcX == dstX) // y축과 평행한 직선 경로일 때
	{
		for(int y = min(srcY, dstY) + 1; y < max(srcY, dstY); y++)
		{
			// 사이에 장애물이 있으면 BLOCKED
			if(this->getPieceAt(srcX, y) != nullptr) return PathState::BLOCKED;
		}
		return PathState::STRAIGHT_LINE;
	}
	else if(abs(srcX - dstX) == abs(srcY - dstY)) // 대각선 경로일 때
	{
		int stepX = srcX < dstX ? 1 : -1;
		int stepY = srcY < dstY ? 1 : -1;
		for(int x = srcX + stepX, y = srcY + stepY;
			x != dstX && y != dstY;
			x += stepX, y += stepY)
		{
			// 사이에 장애물이 있으면 BLOCKED
			if(this->getPieceAt(x, y) != nullptr) return PathState::BLOCKED;
		}
		return PathState::DIAGONAL;
	}
	else // 직선도, 대각선도 아닌 경로일 때 ("점프 경로")
	{
		return PathState::JUMP;
	}
}


bool ChessEngine::movePieceTo(int srcX, int srcY, int dstX, int dstY)
{
	// src에서 dst로 움직일 수 없으면 false
	if(!this->isPieceMovableTo(srcX, srcY, dstX, dstY)) return false;

	ChessPiece* piece = this->getPieceAt(srcX, srcY);
	bool result = piece->moveTo(*this, dstX, dstY);
	if(result)
	{
		this->updateCheckmate();
		this->chessTurn = this->chessTurn == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;
	}

	return result;
}


/**
 * blackCheckmate 변수와 whiteCheckmate 변수를 업데이트시킴.
 * 판이 업데이트될 때마다 이 함수를 호출할 것.
 * 주의: 이 함수는 꽤 오래 걸리기 때문에, 가급적 isCheckmate()를 사용할 것
 */
void ChessEngine::updateCheckmate()
{
	// 체크메이트 여부: 흑 먼저, 그 다음에 백 확인
    this->blackCheckmate = this->calculateCheckmate(PieceColor::BLACK, PieceColor::WHITE);
    this->whiteCheckmate = this->calculateCheckmate(PieceColor::WHITE, PieceColor::BLACK);
}


bool ChessEngine::calculateCheckmate(PieceColor victimColor, PieceColor opponentColor)
{
    // 킹이 어딨는지 확인
	ChessPiece* kingPiece = this->findPiece(PieceType::KING, victimColor);
	if(kingPiece == nullptr) return false;

    // 킹을 찾았을 때, 체스 판을 모두 찾으면서 상대편 말이 나오면
    // 그 말이 킹을 잡을 수 있는지 확인.
	for(ChessPiece* piece : this->pieces)
	{
		if(piece == nullptr) continue;

		// 말 색깔이 상대편 색깔이 아니면 스킵
		if(piece->color != opponentColor) continue;

        // 상대편 말이 킹을 잡을 수 있는지 확인
		if(this->isPieceMovableTo(piece->x, piece->y, kingPiece->x, kingPiece->y)) return true;
	}

	return false;
}


bool ChessEngine::isCheckmate(PieceColor color)
{
    if(color == PieceColor::WHITE) return this->whiteCheckmate;
    else if(color == PieceColor::BLACK) return this->blackCheckmate;
    return false;
}


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
				if(!this->isPieceMovableTo(selX, selY, x, y)) continue;
				printCoverTensor[y][x][0] = '(';
				printCoverTensor[y][x][1] = ')';
			}
		}
	}

	for(ChessPiece* piece : this->pieces)
	{
		if(piece == nullptr) continue;

		int x = piece->x, y = piece->y;
		if(piece->x < 0 || 8 <= piece->x || piece->y < 0 || 8 <= piece->y) continue;

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