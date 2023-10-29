#include "chess_engine.h"


class PawnPiece : public ChessPiece
{
public:
	PawnPiece(int x, int y, PieceColor color)
		: ChessPiece(x, y, PieceType::PAWN, color)
	{}

	bool isMovableTo(ChessEngine &engine, int dstX, int dstY)
	{
		PathState pathState = engine.checkPath(this->x, this->y, dstX, dstY);
		ChessPiece* dstPiece = engine.getPieceAt(dstX, dstY);
		
		if((this->color == PieceColor::BLACK && this->y == 1 && dstY == 3) ||
		   (this->color == PieceColor::WHITE && this->y == 6 && dstY == 4)) // 2칸 앞 이동
		{
			return pathState == PathState::STRAIGHT_LINE && dstPiece == nullptr;
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

	void whenMoved(ChessEngine &engine, int dstX, int dstY) override
	{
		// TODO: 폰이 끝에 도달했을 때도 구현할 것
		
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

	void whenMoved(ChessEngine &engine, int dstX, int dstY) override
	{
		this->didMove = true;
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

	void whenMoved(ChessEngine &engine, int dstX, int dstY)
	{
		this->didMove = true;
	}

	/**
	 * @return 캐슬링할 수 있을 경우 룩의 객체 포인터. 할 수 없을 경우 nullptr
	 * @param dstX 킹이 움직일 자리 (x좌표)
	 * @param dstY 킹이 움직일 자리 (y좌표)
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

	int getCastlingRookDstX(ChessEngine &engine, int kingDstX, RookPiece* castlingRook)
	{
		// 캐슬링 조건이 아닌 상태에서 이 함수를 실행시킬 수 있기 때문에
		// (이 때 castlingRook으로 nullptr가 들어옴) nullptr 체크를 해야 됨
		if(castlingRook == nullptr) return -1;

		if     (kingDstX == 2) return 3; // 퀸 사이드 캐슬링
		else if(kingDstX == 6) return 5; // 킹 사이드 캐슬링
		else return -1; // getCastlingVictim()이 nullptr라면 이게 리턴될 일은 없을거임
	}
};


ChessPiece::ChessPiece(int x_, int y_, PieceType type_, PieceColor color_)
	: x(x_), y(y_), type(type_), color(color_)
{}


ChessPiece::~ChessPiece()
{}


ChessEngine::ChessEngine()
{
	this->resetBoard();
}


ChessEngine::~ChessEngine()
{
	this->clearBoard();
}


void ChessEngine::resetBoard(const char *sequence)
{
	if(strlen(sequence) != 64) return;

	this->clearBoard();
	
	for(int i = 0; i < 64; i++)
	{
		int x = i % 8, y = i / 8;
		char c = sequence[i];

		PieceColor color = (c & 0b00100000) == 0 ? PieceColor::BLACK : PieceColor::WHITE;
		ChessPiece *piece;
		switch(c & 0b01011111)
		{
			case 'P': piece = new PawnPiece(x, y, color);   break;
			case 'R': piece = new RookPiece(x, y, color);   break;
			case 'N': piece = new KnightPiece(x, y, color); break;
			case 'B': piece = new BishopPiece(x, y, color); break;
			case 'Q': piece = new QueenPiece(x, y, color);  break;
			case 'K': piece = new KingPiece(x, y, color);   break;
			default: piece = nullptr;
		}
		this->chessBoard[y][x] = piece;
	}
	this->updateCheckmate();
	
	this->chessTurn = PieceColor::WHITE;
}


/**
 * 체스 판을 게임 시작 상태로 초기화 시켜주는 함수.
 */
void ChessEngine::resetBoard()
{
	this->resetBoard(
		"RNBQKBNR"
		"PPPPPPPP"
		"        "
		"        "
		"        "
		"        "
		"pppppppp"
		"rnbqkbnr"
	);
}


/**
 * 체스 판 위를 싹 다 비워버리는 함수.
 */
void ChessEngine::clearBoard()
{
	for(int y = 0; y < 8; y++) for(int x = 0; x < 8; x++)
	{
		ChessPiece*& piece = this->chessBoard[y][x];
		delete piece;
		piece = nullptr;
	}
}


/**
 * (x, y)에 있는 말을 리턴해주는 함수. 만약 비어있다면 nullptr 리턴.
 */
ChessPiece* ChessEngine::getPieceAt(int x, int y)
{
	return this->chessBoard[y][x];
}


/**
 * 말 종류가 type이고 색깔이 color인 말을 찾아주는 함수. 없다면 nullptr 리턴.
 */
ChessPiece* ChessEngine::findPiece(PieceType type, PieceColor color)
{
	for(int y = 0; y < 8; y++) for(int x = 0; x < 8; x++)
	{
		ChessPiece* piece = this->chessBoard[y][x];
		if(piece == nullptr) continue;
		if(piece->type == type && piece->color == color) return piece;
	}
	return nullptr;
}


/**
 * (x, y)에 있는 말을 없애버리는(=nullptr로 만들어버리는) 함수.
 * 원래 (x, y)에 말이 없었어도 문제 없이 작동함.
 */
void ChessEngine::killPieceAt(int x, int y)
{
	ChessPiece*& piece = this->chessBoard[y][x];
	delete piece;
	piece = nullptr;
}


/**
 * (srcX, srcY)에 있는 말을 (dstX, dstY)로 옮길 수 있는지 알려주는 함수.
 * 원래 (srcX, srcY)에 말이 없었다면 false 리턴.
 * @param checkTurn true일 경우 턴이 맞는지까지 계산함.
 * @param checkCheckmate true일 경우 체크메이트인지까지 계산함.
 */
bool ChessEngine::isPieceMovableTo(int srcX, int srcY, int dstX, int dstY, bool checkTurn, bool checkCheckmate)
{
	if(srcX == dstX && srcY == dstY) return false; // src == dst일 때 false
	if(srcX < 0 || 8 <= srcX || srcY < 0 || 8 <= srcY) return false; // src가 체스 판 밖일 때 false
	if(dstX < 0 || 8 <= dstX || dstY < 0 || 8 <= dstY) return false; // dst가 체스 판 밖일 때 false

	ChessPiece* srcPiece = this->getPieceAt(srcX, srcY);
	// src에 체스 말이 없다면 false
	if(srcPiece == nullptr) return false;

	// 현재 턴과 맞는지 확인
	if(checkTurn && srcPiece->color != this->chessTurn) return false;
	
	ChessPiece* dstPiece = this->getPieceAt(dstX, dstY);
	// dst에 체스 말이 있는데 src와 같은 색깔이라면 false
	if(dstPiece != nullptr && dstPiece->color == srcPiece->color) return false;

	// 움직일 수 있는지 확인 (체크메이트일지는 아직 확인하지 않음)
	bool isMovable = srcPiece->isMovableTo(*this, dstX, dstY);
	if(!isMovable) return false;
	
	// 체크메이트일 경우 false 리턴
	if(checkCheckmate && this->simulateCheckmate(this->chessTurn, srcX, srcY, dstX, dstY)) return false;
	return true;
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


/**
 * (srcX, srcY)에 있는 말을 별도의 확인(isPieceMovableTo 등) 없이 강제로 (dstX, dstY)로 움직임.
 * 만약 (srcX, srcY)에 아무것도 없다면 아무것도 하지 않음.
 * 
 * movePhysicalPieceTo()나 killPhysicalPieceAt()은 부르지 않음.
 * 
 * @return 원래 (dstX, dstY)에 있던 말.
 */
ChessPiece* ChessEngine::forceMovePieceTo(int srcX, int srcY, int dstX, int dstY)
{
	ChessPiece* piece = this->getPieceAt(srcX, srcY);
	if(piece == nullptr) return nullptr;

	ChessPiece* tempPiece = this->chessBoard[dstY][dstX];
	this->chessBoard[dstY][dstX] = piece;
	this->chessBoard[srcY][srcX] = nullptr;
	piece->x = dstX; piece->y = dstY;

	return tempPiece;
}


/**
 * 말을 (srcX, srcY)에서 (dstX, dstY)로 움직일 때, turn이 체크메이트가 되는지 계산하는 함수.
 * 
 * 말을 가상으로 움직여본 후, 체크메이트를 계산하고 다시 원상복구시키기 때문에 계산량이 많음.
 * 되도록이면 자주 실행하지 말 것
 * 
 * @return 체크메이트가 된다면 true, 아니면 false를 리턴함.
 */
bool ChessEngine::simulateCheckmate(PieceColor turn, int srcX, int srcY, int dstX, int dstY)
{
	// (srcX, srcY)에 있는 말(의 포인터)을 가져옴
	ChessPiece* piece = this->getPieceAt(srcX, srcY);

	// 먼저 캐슬링인지 확인함.
	// 캐슬링을 확인하는 이유는 한번에 2개의 말을 움직이는 유일한 움직임이기 때문.
	RookPiece* castlingRook = nullptr;
	int cVSrcX, cVDstX; // cV: castlingVictim; 코드가 너무 길어져서 줄임
	if(piece->type == PieceType::KING)
	{
		KingPiece* castlingKing = static_cast<KingPiece*>(piece);
		castlingRook = castlingKing->getCastlingVictim(*this, dstX, dstY);

		// 캐슬링이 맞을 경우
		if(castlingRook != nullptr)
		{
			// 캐슬링 당하는 룩을 움직임. (이 때 y좌표는 움직이지 않으므로 srcY로 통일)
			cVSrcX = castlingRook->x;
			cVDstX = castlingKing->getCastlingRookDstX(*this, dstX, castlingRook);
			this->forceMovePieceTo(cVSrcX, srcY, cVDstX, srcY);
		}
	}

	// 원래 (dstX, dstY)에 있던 말을 "eatenPiece"에 임시로 저장함.
	// ((dstX, dstY)가 비어있었더라도 상관 없음)
	// 그리고 움직일 말을 (srcX, srcY)에서 (dstX, dstY)로 움직임.
	ChessPiece* eatenPiece = this->forceMovePieceTo(srcX, srcY, dstX, dstY);

	// 체크메이트 여부를 계산함.
	// 움직인 행위를 취소할거기 때문에 prevWhiteCM과 prevBlackCM에 이전 체크메이트 여부를 저장함.
	bool prevWhiteCM = this->whiteCheckmate;
	bool prevBlackCM = this->blackCheckmate;
	this->updateCheckmate();
	bool result = this->isCheckmate(turn);

	// 가상으로 움직인 행위를 취소함.
	this->forceMovePieceTo(dstX, dstY, srcX, srcY);
	this->chessBoard[dstY][dstX] = eatenPiece;

	// 캐슬링이었을 경우
	if(castlingRook != nullptr)
	{
		this->forceMovePieceTo(cVDstX, srcY, cVSrcX, srcY);
	}

	// 체크메이트 여부를 원래대로 바꾼 후 리턴함
	this->whiteCheckmate = prevWhiteCM;
	this->blackCheckmate = prevBlackCM;
	return result;
}


bool ChessEngine::movePieceTo(int srcX, int srcY, int dstX, int dstY)
{
	// src에서 dst로 움직일 수 없으면 false 리턴
	if(!this->isPieceMovableTo(srcX, srcY, dstX, dstY, true, true)) return false;

	// (srcX, srcY)에 있는 말(의 포인터)을 가져옴
	ChessPiece* piece = this->getPieceAt(srcX, srcY);

	// 캐슬링인지 확인함.
	RookPiece* castlingRook = nullptr;
	int cVSrcX, cVDstX; // cV: castlingVictim; 코드가 너무 길어져서 줄임
	if(piece->type == PieceType::KING)
	{
		KingPiece* castlingKing = static_cast<KingPiece*>(piece);
		castlingRook = castlingKing->getCastlingVictim(*this, dstX, dstY);
	}

	// 말을 이동시킴.
	ChessPiece* eatenPiece = this->forceMovePieceTo(srcX, srcY, dstX, dstY);

	// 만약 eatenPiece가 있었다면 제거함. (무조건 movePhysicalPieceTo 이전에 실행해야 함)
	if(eatenPiece != nullptr)
	{
		delete eatenPiece;
		killPhysicalPieceAt(dstX, dstY);
	}

	// whenMoved()를 실행함.
	piece->whenMoved(*this, dstX, dstY);
	if(castlingRook != nullptr)
	{
		castlingRook->whenMoved(*this, cVDstX, srcY);
	}
	movePhysicalPieceTo(srcX, srcY, dstX, dstY);

	// 체크메이트 여부를 다시 계산함.
	this->updateCheckmate();

	// 턴을 바꿈
	this->chessTurn = this->chessTurn == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;
	return true;
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
	for(int y = 0; y < 8; y++) for(int x = 0; x < 8; x++)
	{
		// (x, y)에 말이 없다면 스킵
		ChessPiece* piece = this->chessBoard[y][x];
		if(piece == nullptr) continue;

		// 말 색깔이 상대편 색깔이 아니면 스킵
		if(piece->color != opponentColor) continue;

        // 상대편 말이 킹을 잡을 수 있는지 확인
		// 이 때 checkTurn=false, checkCheckmate=false로 잡아야 됨 (아니면 무한루프에 빠짐)
		if(this->isPieceMovableTo(piece->x, piece->y, kingPiece->x, kingPiece->y, false, false)) return true;
	}

	return false;
}


bool ChessEngine::isCheckmate(PieceColor color)
{
    if(color == PieceColor::WHITE) return this->whiteCheckmate;
    else if(color == PieceColor::BLACK) return this->blackCheckmate;
    return false;
}