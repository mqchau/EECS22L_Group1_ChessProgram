#include "Model.h"

ChessBoard * Model_Initialize(void){
	return ChessBoard_Initialize();
}

/* move piece to next location */
ChessBoard * Model_PerformMove(ChessBoard * board, ChessMoveList * moveList, ChessMove * move)
{	
  ChessMove *rookMove;

	/*if the move piece is a pawn, set first move flag to false*/
	if (move->MovePiece->Type == Pawn){
		move->MovePiece->PawnMoveFirstFlag = False;
	}

	move->MoveType = Model_GetMoveType(board, move);

	if(move->MoveType == Castling) {
	  /* piece to move, moved to next coordinate */
	  move->StartPosition->Piece->Coordinate = move->NextPosition;
	  move->NextPosition->Piece = move->MovePiece;	
	
	  /* delete piece pointer at previous location */
	  move->StartPosition->Piece = NULL;
	  
	  /* since castling requires two moves, we create 'rookMove' based on king/queen-side */
	  if(move->NextPosition->File == 1) {
	    rookMove->MovePiece = board->Board[move->NextPosition->Rank][0]->Piece;
	    rookMove->StartPosition = board->Board[move->NextPosition->Rank][0];
	    rookMove->NextPosition = board->Board[move->NextPosition->Rank][2];
	    rookMove->MoveType = Castling;

	    rookMove->StartPosition->Piece->Coordinate = rookMove->NextPosition;
	    rookMove->NextPosition->Piece = rookMove->MovePiece;

	    rookMove->StartPosition->Piece = NULL;
	  }
	  if(move->NextPosition->File == 6) {
	    rookMove->MovePiece = board->Board[move->NextPosition->Rank][7]->Piece;
	    rookMove->StartPosition = board->Board[move->NextPosition->Rank][7];
	    rookMove->NextPosition = board->Board[move->NextPosition->Rank][5];
	    rookMove->MoveType = Castling;

	    rookMove->StartPosition->Piece->Coordinate = rookMove->NextPosition;
	    rookMove->NextPosition->Piece = rookMove->MovePiece;

	    rookMove->StartPosition->Piece = NULL;
	  }
	    

	  /* update move list */
	  ChessMoveList_AppendMove(moveList, move);
	  ChessMoveList_AppendMove(moveList, rookMove);

	}

	else {	
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
	  ChessMoveList_AppendMove(moveList, move);
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

ChessMoveTypeEnum Model_GetMoveType(ChessBoard * board, ChessMove *move) {
  
  if(move->MovePiece->Type == Pawn) {
    /* for transformation, we want to check if pawn is at rank 0 or 7 */
    if(move->NextPosition->Rank == 0 || move->NextPosition->Rank == 7) {
      return Transformation;
    }
    /* for en passant, check if pawn is moving diagonally for capture */
    if(move->NextPosition->File != move->StartPosition->File) {
      /* check if there is a pawn diagonally for it to capture */
      /* if not, then it is a special move */
      if(move->NextPosition->Piece == NULL) {
	return EnPassant;
      }
    }
  }

  /* for castling, check if king is moving more than one space over */
  if(move->MovePiece->Type == King) {
    if(move->StartPosition->File == 4) {
      if(move->NextPosition->File == 6 || move->NextPosition->File == 1) {
	return Castling;
      }
    }
  }

  return Normal;
    
}

ChessCoordinateList * Model_GetAllLegalCoordinate( ChessBoard * board, ChessPlayer * player, ChessPlayer * PlayerInTurn)
{
	int i = 0;
	
	/* a flag for first coordinate list; making sure the first piece that is going to be store */
	/* in the list is not dead */
	int firstListPiece = 0;
	
	/* create a permanent coordinate list to be return */
	ChessCoordinateList *newChessCoordinateList1;
	/* create a temp coordinate list to be append */
	ChessCoordinateList *newChessCoordinateList2;

	/* storing the value of the permanent list */
	while (i < 16 && firstListPiece == 0)
	{	
		/* making sure the piece is alive before storing */
		if (player->Pieces[i]->AliveFlag == True)
		{
			newChessCoordinateList1 = Model_GetLegalCoordinates(board, player->Pieces[i], PlayerInTurn);
			firstListPiece = 1;
		}
		i++;
	}
	
	/* for loop to store the remaining pieces into a temp list, then appending new coordinate into 
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
			if (False/*replace this with your boolean return*/){
				output = ChessCoordinateList_RemoveAtNode(output, node1);
			}
			node1 = node2;
		}
	}
	
	return output;
}

/* uses GetLegalCoordinates */
/* see if move is legal */
Boolean Model_CheckLegalMove(ChessBoard * board, ChessMove * moveTo)
{
	return True;
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
