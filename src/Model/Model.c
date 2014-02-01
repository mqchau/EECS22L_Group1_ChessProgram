#include "Model.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ChessBoard * Model_Initialize(void){
	return ChessBoard_Initialize();
}

/* move piece to next location */
ChessBoard * Model_PerformMove(ChessBoard * board, ChessMoveList * moveList, ChessMove * move)
{	
	/*if the move piece is a pawn, set first move flag to false*/
	if (move->MovePiece->Type == Pawn){
		move->MovePiece->PawnMoveFirstFlag = False;
	}
	
	/* the place to move to has an enemy piece */
	if (move->NextPosition->Piece != NULL)
	{
		/* send to grave yard*/
		move->CapturePiece = move->NextPosition->Piece;
		/* kill it */
		move->NextPosition->Piece->AliveFlag = False;
		/* set that dead piece coordinate to null */
		move->NextPosition->Piece->Coordinate = NULL;
		
		/* it is dead */
		move->CaptureFlag = True;
	}
	/* piece to move, moved to next coordinate */
	move->StartPosition->Piece->Coordinate = move->NextPosition;
	move->NextPosition->Piece = move->MovePiece;	
	
	/* delete piece pointer at previous location */
	move->StartPosition->Piece = NULL;

	/* update move list */
	ChessMoveNode * newMoveNode = malloc(sizeof(ChessMoveNode));
	assert (newMoveNode);

	/* point new List to old list */
	newMoveNode->List = moveList;

	/* point new move to move */
	newMoveNode->Move = move;
	
	/* set next node to null */
	newMoveNode->NextNode = NULL;

	/* list is empty */
	if (moveList ->FirstNode == NULL)
	{
		moveList->FirstNode = newMoveNode;
		moveList->LastNode = newMoveNode;
		newMoveNode->PrevNode = NULL;
	}

	/* list is not empty */
	else
	{
		moveList->LastNode->NextNode = newMoveNode;
		newMoveNode->PrevNode = moveList->LastNode;
		moveList->LastNode = newMoveNode;
	}
	return board;
}

ChessBoard * Model_UndoLastMove(ChessBoard * board, ChessMoveList * moveList)
{

	/*COUNT NUMBER OF MOVE DONE B4 doing this: IN CASE GAME JUST  STARTED*/
	if (ChessMoveList_Count(moveList) < 3) return board;
      int i = 0;
      /* move to delete */
      ChessMoveNode * tempNode;
      
      /* need to move back twice */
      for (i = 0; i < 2; i ++)
      {	tempNode = moveList->LastNode;
		/* moving back a position (1 undo) */
		tempNode->Move->MovePiece->Coordinate = tempNode->Move->StartPosition;
		tempNode->Move->StartPosition->Piece = tempNode->Move->MovePiece;
		
		/* need to restore a piece from the graveyard */
		if (tempNode->Move->CaptureFlag)
		{
		/* bring back the dead */
		tempNode->Move->NextPosition->Piece = tempNode->Move->CapturePiece;
		tempNode->Move->CapturePiece->Coordinate = tempNode->Move->NextPosition;
		}
		/*nothing to restore from graveyard */
		else
		{
		tempNode->Move->NextPosition->Piece = NULL;
		}
	
		moveList = ChessMoveList_PopLastMove(moveList);
#if 0
		/*breaking the forward link of previous node*/
		tempNode->PrevNode->NextNode = NULL;
		/*moving the node back one */
		tempNode = tempNode->PrevNode;
		/*update the last node */
		moveList->LastNode = tempNode;

		free(tempNode->NextNode);
#endif
      }
      
      return board;
}
ChessCoordinateList * Model_GetAllLegalCoordinate( ChessBoard * board, ChessPlayer * player, ChessPlayer * PlayerInTurn)
{
	int i = 0;
	
	/* a flag for first coordinate list; making sure the first piece that is going to be store */
	/* in the list is not dead */
	int firstListPiece = 0;
	
	/* create a permanent coordinate list that is initually empty to be return */
	ChessCoordinateList *newChessCoordinateList1 = ChessCoordinateList_Initialize();
	/* create a temp coordinate list to be append */
	ChessCoordinateList *newChessCoordinateList2;

	
	/* for loop to store the  pieces into a temp list, then appending new coordinate into 
	the permanent list */
	while (i < 16)
	{	
		/* making sure the piece is alive before storing */
		if (player->Pieces[i]->AliveFlag == True)
		{
			/* storing coordinate into the temp list */
			newChessCoordinateList2 = Model_GetLegalCoordinates(board, player->Pieces[i], PlayerInTurn);
			
			/* appending the two list so there is no duplicate coordinate */
			newChessCoordinateList1 = ChessCoordinateList_AppendNoRedundancy(newChessCoordinateList1, newChessCoordinateList2);
		}
		i++;
	}
	return newChessCoordinateList1;
}

Boolean Model_CheckCheckedPosition(ChessBoard * board, ChessPlayer * player)
{
	/* grab list of legal move of other player */
	ChessCoordinateList * newList = Model_GetAllLegalCoordinate(board, player->OtherPlayer, player);
	
	/*get the king of current player*/
	ChessPiece * king = ChessPlayer_GetChessPiece(player, King, 0);
	
	if (ChessCoordinateList_CheckRedundancy(newList, king->Coordinate)){
		ChessCoordinateList_Free(newList);
		return True;
	}
	ChessCoordinateList_Free(newList);
	return False;

}

Boolean Model_CheckCheckmate(ChessBoard * board, ChessPlayer * player)
{
	
	ChessCoordinateList * CurrPlayerPossibleCoords = Model_GetAllLegalCoordinate(board, player, player);
	if (Model_CheckCheckedPosition(board, player) &&  !CurrPlayerPossibleCoords->FirstNode)
	{
		ChessCoordinateList_Free(CurrPlayerPossibleCoords);
		return True;
	}
	ChessCoordinateList_Free(CurrPlayerPossibleCoords);
	return False;
}

Boolean Model_CheckStalemate(ChessBoard * board, ChessPlayer * player)
{	ChessCoordinateList * CurrPlayerPossibleCoords = Model_GetAllLegalCoordinate(board, player, player);
	if (!Model_CheckCheckedPosition(board, player) &&  !CurrPlayerPossibleCoords->FirstNode)
	{
		ChessCoordinateList_Free(CurrPlayerPossibleCoords);
		return True;
	}
	ChessCoordinateList_Free(CurrPlayerPossibleCoords);
	return False;
}

ChessCoordinateList * Model_GetLegalCoordinates(ChessBoard *chessboard, ChessPiece *piece, ChessPlayer *playerinturn) {
	ChessCoordinateList *output = ChessCoordinateList_Initialize();
	ChessCoordinateList *OpponentLegalMoves;
	ChessCoordinate * target_coor = NULL;
	ChessCoordinate * curr_coor = NULL;
	
	
	int targetRank, targetFile;
	int dir_index = 0;
	int Rank_Offset8[8] = {1,1,1,0,0,-1,-1,-1};
	int File_Offset8[8] = {-1,0,1,-1,1,-1,0,1};
	int Rank_Offset_Bish[4] = {1,1,-1,-1};
	int File_Offset_Bish[4] = {-1,1,-1,1};
	int Rank_Offset_Rook[4] = {1,-1,0,0};
	int File_Offset_Rook[4] = {0,0,1,-1};
	int inDanger = 0;
	
	Boolean StopFlag;
	
	switch(piece->Type)
	  { 
	  case Pawn:
	      
		if(piece->Player->PlayerColor == White) {
			/* check rank+1 to see if empty */
			targetRank = 1 + piece->Coordinate->Rank;
			targetFile = piece->Coordinate->File;
			if(targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
					if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
						output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
					}
			}
			
			/* check rank+1, file-1 to see if capture */
			targetRank = piece->Coordinate->Rank + 1;
			targetFile = piece->Coordinate->File - 1;
			if(targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
					if(chessboard->Board[targetRank][targetFile]->Piece != NULL && chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
						output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
					}
			}
			
			
			/* check rank+1, file+1 to see if capture */
			targetRank = piece->Coordinate->Rank + 1;
			targetFile = piece->Coordinate->File + 1;
			if(targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
					if(chessboard->Board[targetRank][targetFile]->Piece != NULL && chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
						output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
					}
			}
			
			
			/* if hasn't moved yet, check rank+2 if empty */
			if(piece->PawnMoveFirstFlag) {
				targetRank = piece->Coordinate->Rank + 2;
				targetFile = piece->Coordinate->File;
				if(targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
					if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
						output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
					}
				}
			}

		}

		if(piece->Player->PlayerColor == Black) {
			/* check rank-1 to see if empty */
			targetRank = -1 + piece->Coordinate->Rank;
			targetFile = piece->Coordinate->File;
			if(targetRank >= 0 && targetFile >= 0 && targetFile <= 7) {
					if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
						output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
					}
			}
			
			/* check rank-1, file-1 to see if capture */
			targetRank = piece->Coordinate->Rank - 1;
			targetFile = piece->Coordinate->File - 1;
			if(targetRank >= 0 && targetFile >= 0 && targetFile <= 7) {
					if(chessboard->Board[targetRank][targetFile]->Piece != NULL && chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
						output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
					}
			}
			
			/* check rank-1, file+1 to see if capture */
			targetRank = piece->Coordinate->Rank - 1;
			targetFile = piece->Coordinate->File + 1;
			if(targetRank >= 0 && targetFile >= 0 && targetFile <= 7) {
					if(chessboard->Board[targetRank][targetFile]->Piece != NULL && chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
						output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
					}
			}
			
			/* if hasn't moved yet, check rank-2 if empty */
			if(piece->PawnMoveFirstFlag) {
				targetRank = piece->Coordinate->Rank - 2;
				targetFile = piece->Coordinate->File;
				if(targetRank >= 0 && targetFile >= 0 && targetFile <= 7) {
					if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
						output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
					}
				}
			}
			
		}
		break;

	case Knight: 
		targetRank = 1 + piece->Coordinate->Rank;
		targetFile = 2 + piece->Coordinate->File;
		if(targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
			if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
			}
			else if(chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
				/* capture piece */
			}
		}
		targetRank = 2 + piece->Coordinate->Rank;
		targetFile = 1 + piece->Coordinate->File;
		if(targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
			if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
			}
			else if(chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
				/* capture piece */
			}
		}
		
		targetRank = -1 + piece->Coordinate->Rank;
		targetFile = 2 + piece->Coordinate->File;
		if(targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
			if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
			}
			else if(chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
				/* capture piece */
			}
		}
		targetRank = -2 + piece->Coordinate->Rank;
		targetFile = 1 + piece->Coordinate->File;
		if(targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
			if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
			}
			else if(chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
				/* capture piece */
			}
		}
		
		targetRank = 1 + piece->Coordinate->Rank;
		targetFile = -2 + piece->Coordinate->File;
		if(targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
			if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
			}
			else if(chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
				/* capture piece */
			}
		}
		targetRank = 2 + piece->Coordinate->Rank;
		targetFile = -1 + piece->Coordinate->File;
		if(targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
			if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
			}
			else if(chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
				/* capture piece */
			}
		}
		
		targetRank = -1 + piece->Coordinate->Rank;
		targetFile = -2 + piece->Coordinate->File;
		if(targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
			if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
			}
			else if(chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
				/* capture piece */
			}
		}
		targetRank = -2 + piece->Coordinate->Rank;
		targetFile = -1 + piece->Coordinate->File;
		if(targetRank >= 0 && targetRank <= 7 && targetFile >= 0 && targetFile <= 7) {
			if(chessboard->Board[targetRank][targetFile]->Piece == NULL) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
			}
			else if(chessboard->Board[targetRank][targetFile]->Piece->Player != piece->Player) {
				output = ChessCoordinateList_AppendCoord(output,chessboard->Board[targetRank][targetFile]);
				/* capture piece */
			}
		}
	
		break;

	case Queen:
	
		curr_coor = piece->Coordinate;
		for(dir_index = 0; dir_index < 8; dir_index++)
		{
			target_coor = piece->Coordinate;
			target_coor = ChessCoordinate_Offset(target_coor, Rank_Offset8[dir_index], File_Offset8[dir_index]);
			StopFlag = False;
			while (target_coor && !StopFlag)
			{	  

				if (target_coor->Piece == NULL)
					output = ChessCoordinateList_AppendCoord(output,target_coor);  
				else if (target_coor->Piece->Player == piece->Player)
					StopFlag = True;
				else {
					output = ChessCoordinateList_AppendCoord(output,target_coor);  
					StopFlag = True;
				}
				target_coor = ChessCoordinate_Offset(target_coor, Rank_Offset8[dir_index], File_Offset8[dir_index]);  
				
			}
		}
	
		break;
	case Bishop:
	
		curr_coor = piece->Coordinate;
		for(dir_index = 0; dir_index < 4; dir_index++)
		{
			target_coor = piece->Coordinate;
			target_coor = ChessCoordinate_Offset(target_coor, Rank_Offset_Bish[dir_index], File_Offset_Bish[dir_index]);
			StopFlag = False;
			while (target_coor && !StopFlag)
			{	  
				
				if (target_coor->Piece == NULL){
					output = ChessCoordinateList_AppendCoord(output,target_coor);  
				} else if (target_coor->Piece->Player == piece->Player){
					StopFlag = True;
				} else {
					output = ChessCoordinateList_AppendCoord(output,target_coor);  
					StopFlag = True;
				}
				target_coor = ChessCoordinate_Offset(target_coor, Rank_Offset_Bish[dir_index], File_Offset_Bish[dir_index]);  
					
			}
		}
		
		break;
	  case Rook:	      
	      curr_coor = piece->Coordinate;		
	      for(dir_index = 0; dir_index < 4; dir_index++)
		{
			target_coor = piece->Coordinate;
			target_coor = ChessCoordinate_Offset(target_coor, Rank_Offset_Rook[dir_index], File_Offset_Rook[dir_index]);
			StopFlag = False;
		  while (target_coor && !StopFlag)
		    {	  
			  
			  if (target_coor->Piece == NULL){
				output = ChessCoordinateList_AppendCoord(output,target_coor);  
			  } else if (target_coor->Piece->Player == piece->Player){
				StopFlag = True;
			  } else {
				output = ChessCoordinateList_AppendCoord(output,target_coor);  
				StopFlag = True;
			  }
			  target_coor = ChessCoordinate_Offset(target_coor, Rank_Offset_Rook[dir_index], File_Offset_Rook[dir_index]);  
			  	    
		    }
		}
	      
		break;
	case King: 
	      curr_coor = piece->Coordinate;
	            
		      
		if(piece->Player == playerinturn){
			OpponentLegalMoves = Model_GetAllLegalCoordinate(chessboard, piece->Player->OtherPlayer, playerinturn);				
			for(dir_index = 0; dir_index < 8; dir_index++)
			{
			
				target_coor = piece->Coordinate;				
				target_coor = ChessCoordinate_Offset(target_coor, Rank_Offset8[dir_index], File_Offset8[dir_index]);							
				
				if (!target_coor) continue;
				else if (target_coor->Piece){
					if (target_coor->Piece->Player == piece->Player)
						continue;
				}
			
				
				ChessCoordinateNode * checkSpace = OpponentLegalMoves->FirstNode;
				inDanger = 0;
				while(checkSpace) {
					if(target_coor == checkSpace->Coordinate) {
						inDanger = 1;
						break;
					} else {
					checkSpace = checkSpace->NextNode;
					}
				}
				
				if (inDanger == 0 || !checkSpace){		/*if the opponent can't make any move as well*/
					output = ChessCoordinateList_AppendCoord(output,target_coor);  
				}
						
			}
			ChessCoordinateList_Free(OpponentLegalMoves);
		} else {			
			for(dir_index = 0; dir_index < 8; dir_index++){
				target_coor = piece->Coordinate;
				target_coor = ChessCoordinate_Offset(target_coor, Rank_Offset8[dir_index], File_Offset8[dir_index]);
				StopFlag = False;
					
				if (!target_coor){
					continue;
				}else if (target_coor->Piece == NULL){
					output = ChessCoordinateList_AppendCoord(output,target_coor);  
				} else if (target_coor->Piece->Player == piece->Player){
					
				} else {
					output = ChessCoordinateList_AppendCoord(output,target_coor);  
					
				}					
						
				
			}
		}
		
	  break;
	}
	
	/* right here, before return the coord list, we will only select coordinate that won't get our king killed
	 * but that happens only when we're in defensive mode. When it's not our turn (we're in offensive mode, 
	 * don't need to care about this   */
	if (piece->Player == playerinturn && output->FirstNode){
		ChessCoordinateNode * node1 = output->FirstNode, * node2;
		while (node1){
			node2 = node1->NextNode;
			ChessMove * moveTo = ChessMove_Initialize();
			moveTo->MovePiece = piece;
			moveTo->StartPosition = piece->Coordinate;
			moveTo->NextPosition = node1->Coordinate;
			if (Model_CheckLegalMove(chessboard, moveTo)){
				output = ChessCoordinateList_RemoveAtNode(output, node1);
			}
			node1 = node2;
		}
	}
	
	return output;
}

int writeToLogFile(char fname[100], ChessMoveList * moveList)
{
	/* creating the file */
	FILE *File;
	
	/* file type */
	char ftype[] = ".txt";
	
	/* a copy of the file name */
	char fname_tmp[100];  
	
	/* node pointer to traverses the list */
	ChessMoveNode * temp = moveList->FirstNode;
	
	/* move counter */
	int counter = 1;
	
	/* string to convert the enum type into a char[] type */
	char playerColor[10] = "";
	char pieces[10] = "";
	char captureType[10] = "";
	char transformType[10] = "";
	char file1;
	char file2;
	int castlingFLag = 0;

	/* copy file name to new file */
	strcpy(fname_tmp, fname);
	
	/* concat the file type to the name */
	strcat(fname_tmp, ftype);

	/* open the file */
	File = fopen(fname_tmp, "w");
	
	/* file could not be open */
	if (!File) 
	{
	  printf("Cannot open file \"%s\" for writing!\n", fname);
	  return 1;
	}
	
	/* going though the list and printing the move */
	while (temp)
	{
		/* getting the player color */
		switch(temp->Move->MovePiece->Player->PlayerColor)
		{
		  case White:
		    strcpy(playerColor,  "White");
		    break;
		  case Black:
		    strcpy(playerColor,  "Black");
		    break;
		}
		
		/* getting the move piece */
		switch(temp->Move->MovePiece->Type)
		{
		  case Pawn:
		    strcpy(pieces,  "Pawn");
		    break;
		  case Queen:
		    strcpy(pieces,  "Queen");
		    break;
		  case King:
		    strcpy(pieces,  "King");
		    break;
		  case Rook:
		    strcpy(pieces,  "Rook");
		    break;
		  case Bishop:
		    strcpy(pieces,  "Bishop");
		    break;
		  case Knight:
		    strcpy(pieces,  "Knight");
		    break;
		}
		
		/* getting the file position */
		switch(temp->Move->StartPosition->File)
		{
		  case 0:
		    file1 = 'a';
		    break;
		  case 1:
		    file1 = 'b';
		    break;
		  case 2:
		    file1 = 'c';
		    break;
		  case 3:
		    file1 = 'd';
		    break;
		  case 4:
		    file1 = 'e';
		    break;
		  case 5:
		    file1 = 'f';
		    break;
		  case 6:
		    file1 = 'g';
		    break;
		  case 7:
		    file1 = 'h';
		    break; 
		}
		
		/* getting the ending file positon */
		switch(temp->Move->NextPosition->File)
		{
		  case 0:
		    file2 = 'a';
		    break;
		  case 1:
		    file2 = 'b';
		    break;
		  case 2:
		    file2 = 'c';
		    break;
		  case 3:
		    file2 = 'd';
		    break;
		  case 4:
		    file2 = 'e';
		    break;
		  case 5:
		    file2 = 'f';
		    break;
		  case 6:
		    file2 = 'g';
		    break;
		  case 7:
		    file2 = 'h';
		    break; 
		}
		
		/*this is the castling move, it is basically two move in one */
		if (temp->Move->MoveType == Castling)
		{
		    /* first move of the castlilng */
		    if (castlingFLag == 0)
		    {
		      /* print castling only once for the two castling move */
		      fprintf(File, "Castling"); 
		      castlingFLag = 1;
		    }
		    /* second move of the castling */
		    else
		    {
		      counter--;
		      castlingFLag = 0;
		    }
		}
		else if (temp->Move->MoveType == Normal)
		{
		  fprintf(File, "Normal");
		}
		else if (temp->Move->MoveType == EnPassant)
		{
		  fprintf(File, "En Passant");
		}
		else
		{
		  fprintf(File, "Transformation");
		}
		
		/* print the move */
		fprintf(File, "  Move #%d: %s %s move from %c%c to %c%c", counter, playerColor, pieces, file1, temp->Move->StartPosition->Rank, file2, temp->Move->NextPosition->Rank);

		
		/* print what the pawn transform to */
		if (temp->Move->MoveType == Transformation)
		{
		  switch (temp->Move->Transform_IntoType)
		  {
		    case Rook:
		      strcpy(transformType,  "Rook");
		      break;
		    case Queen:
		      strcpy(transformType,  "Queen");
		      break;
		    case Bishop:
		      strcpy(transformType,  "Bishop");
		      break;
		    case Knight:
		      strcpy(transformType,  "Knight");
		      break;
		    case Pawn:
		      break;
		    case King:
		      break;
		  }
		  fprintf(File, "    Pawn transforms into %s", transformType);
		}
		  
		/* print the capture piece */
		if (temp->Move->CaptureFlag == True)
		{
		  switch(temp->Move->CapturePiece->Type)
		    {
		      case Pawn:
			strcpy(captureType,  "Pawn");
			break;
		      case Queen:
			strcpy(captureType,  "Queen");
			break;
		      case King:
			strcpy(captureType,  "King");
			break;
		      case Rook:
			strcpy(captureType,  "Rook");
			break;
		      case Bishop:
			strcpy(captureType,  "Bishop");
			break;
		      case Knight:
			strcpy(captureType,  "Knight");
			break;
		    }
		  /* a somewhat special message if the queen is capture */
		  if (temp->Move->CapturePiece->Type == Queen)
		  {
		    fprintf(File, "    %s player has captured the graceful %s", playerColor, captureType);
		  }

		  /* capture message for everything else besides king */
		  else
		  {
		    fprintf(File, "    %s has captured a %s", playerColor, captureType);
		  }
		}
			
		/* advance the node */
		temp = temp->NextNode;
		
		/* increase the counter */
		counter++;
	}
	
	/* un able to open or an error */
	if (ferror(File)) 
	{
		printf("\nFile error while writing to file!\n");
		return 2;
	}
	
	/* close the file */
	fclose(File);
	
	/*print that the file was saved successfully */
	printf("%s was saved successfully. \n", fname_tmp);

	return (0);
}

/* uses GetLegalCoordinates */
/* see if move is legal */
Boolean Model_CheckLegalMove(ChessBoard * board, ChessMove * moveTo)
{
	Boolean checkKing = False;
	ChessMoveList * moveList = ChessMoveList_Initialize();
	
	/* create a temporary board */
	ChessBoard * tempBoard =  ChessBoard_InitializeEmpty();
	
	/* duplicate chess buard */
	tempBoard = Model_duplicateChessBoard(tempBoard, board);
	
	/* perform the "temp" move */
	tempBoard = Model_PerformMove(tempBoard, moveList, moveTo);
	
	/* check to see if the king is in check */
	checkKing = !Model_CheckCheckedPosition(tempBoard, moveTo->MovePiece->Player);

	/* free everything */
	ChessMoveList_Free(moveList);
	ChessBoard_Free(tempBoard);
	
	return checkKing;
}

ChessBoard * Model_duplicateChessBoard(ChessBoard * tempBoard, ChessBoard * oldboard)
{
  int rank, file;
  int counter = 0;
  
  /* player property copy */
  tempBoard->WhitePlayer->AIDifficulty = oldboard->WhitePlayer->AIDifficulty;
  tempBoard->WhitePlayer->PlayerControl = oldboard->WhitePlayer->PlayerControl;
  
  tempBoard->BlackPlayer->AIDifficulty = oldboard->BlackPlayer->AIDifficulty;
  tempBoard->BlackPlayer->PlayerControl = oldboard->BlackPlayer->PlayerControl;
  
   /* pieces property copy */
  while (counter < 16)
  {
    tempBoard->WhitePlayer->Pieces[counter]->AliveFlag = oldboard->WhitePlayer->Pieces[counter]->AliveFlag;

    tempBoard->WhitePlayer->Pieces[counter]->PawnMoveFirstFlag = oldboard->WhitePlayer->Pieces[counter]->PawnMoveFirstFlag;
    
    /* link between coordinate and pieces property copy */
    if (oldboard->WhitePlayer->Pieces[counter]->Coordinate)
    {
      rank = oldboard->WhitePlayer->Pieces[counter]->Coordinate->Rank;
      file = oldboard->WhitePlayer->Pieces[counter]->Coordinate->File;
      
      tempBoard->WhitePlayer->Pieces[counter]->Coordinate = tempBoard->Board[rank][file];
      tempBoard->Board[rank][file]->Piece = tempBoard->WhitePlayer->Pieces[counter];
    }
    
    /* BLACK */
    tempBoard->BlackPlayer->Pieces[counter]->AliveFlag = oldboard->BlackPlayer->Pieces[counter]->AliveFlag;

    tempBoard->BlackPlayer->Pieces[counter]->PawnMoveFirstFlag = oldboard->BlackPlayer->Pieces[counter]->PawnMoveFirstFlag;
    
    /* link between coordinate and pieces property copy */
    if (oldboard->BlackPlayer->Pieces[counter]->Coordinate)
    {
      rank = oldboard->BlackPlayer->Pieces[counter]->Coordinate->Rank;
      file = oldboard->BlackPlayer->Pieces[counter]->Coordinate->File;
      
      tempBoard->BlackPlayer->Pieces[counter]->Coordinate = tempBoard->Board[rank][file];
      tempBoard->Board[rank][file]->Piece = tempBoard->BlackPlayer->Pieces[counter];
    }
    counter++;
  }

  return tempBoard;
}

void Model_CleanUp(ChessBoard * CurrBoard, ChessMoveList * MoveList){
	ChessMoveNode * MoveNode1, * MoveNode2;
	MoveNode1 = MoveList->FirstNode;
	
	while (MoveNode1){
		MoveNode2 = MoveNode1->NextNode;
		free(MoveNode1->Move);
		free(MoveNode1);
		MoveNode1 = MoveNode2;
	}
		
	free(MoveList);
	
	ChessBoard_Free(CurrBoard);
	
}

ChessMove * Model_GetBestMove(ChessBoard * MainBoard, ChessPlayer * PlayerInTurn){
	return NULL;
}
