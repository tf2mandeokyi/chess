#pragma once

#include <iostream>
#include <string.h>
#include "chess_physical.h"

int min(int a, int b) { return a > b ? b : a; }
int max(int a, int b) { return a > b ? a : b; }
int abs(int a) { return a < 0 ? -a : a; }

class ChessPiece;
class ChessEngine;


enum class PieceType : char
{
	PAWN='P', ROOK='R', KNIGHT='N', BISHOP='B', QUEEN='Q', KING='K', 
};

enum class PieceColor
{
	BLACK='0', WHITE='1'
};

enum PathState
{
	STRAIGHT_LINE, DIAGONAL, JUMP, BLOCKED
};


class ChessPiece
{
public:
	int x, y;
	PieceType type;
	PieceColor color;

	ChessPiece(int x_, int y_, PieceType type_, PieceColor color_);
	~ChessPiece();

	virtual bool isMovableTo(ChessEngine&, int dstX, int dstY) = 0;

	/**
	 * 자기 자신이 움직여졌을 때 실행되는 함수.
	 * 기본적으로 아무것도 하지 않음.
	 */
	virtual void whenMoved(ChessEngine&, int dstX, int dstY)
	{}
};


class ChessEngine
{
public:
	ChessEngine();
	~ChessEngine();

	void resetBoard();
	void resetBoard(const char *sequence);
	void clearBoard();

	ChessPiece* getPieceAt(int x, int y);
	ChessPiece* findPiece(PieceType type, PieceColor color);
	void killPieceAt(int x, int y);

	bool isPieceMovableTo(int srcX, int srcY, int dstX, int dstY, bool checkTurn, bool checkCheckmate);
	PathState checkPath(int srcX, int srcY, int dstX, int dstY);
	bool isCheckmate(PieceColor color);
	bool simulateCheckmate(PieceColor turn, int srcX, int srcY, int dstX, int dstY);

	ChessPiece* forceMovePieceTo(int srcX, int srcY, int dstX, int dstY);
	bool movePieceTo(int srcX, int srcY, int dstX, int dstY);
	void printBoard(std::ostream& out, int selX, int selY);

private:
	ChessPiece* chessBoard[8][8];
	PieceColor chessTurn;
	bool whiteCheckmate, blackCheckmate;

	void updateCheckmate();
	bool calculateCheckmate(PieceColor victimColor, PieceColor opponentColor);
};
